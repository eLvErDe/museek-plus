#include <system.h>
#include <fam.h>

#include <muscan/scanner.hh>
#include <Muhelp/Muconf.hh>
#include <NewNet/nnlog.h>

#include <iostream>
#include <cstdlib>
#include <algorithm>

using std::string;
using std::map;
using std::cout;
using std::endl;
using std::cerr;
using std::vector;

class FAMDirScanner;

class FAMHandler
{
public:
	FAMHandler(const string& config_file, bool doBuddy, bool doReload);
	~FAMHandler();
	
	int load();
	int run();
	void save();
	
	void remove(FAMDirScanner *);
	
private:
	string shares, state;
	FAMConnection fc;
	vector<FAMDirScanner *> nodes, pending;
	time_t save_at;
	FAMDirScanner *root;
	bool m_doReload;
	
	void add(FAMDirScanner *);
	void exists(FAMDirScanner *ds, const char *filename, bool force_file);
	void deleted(FAMDirScanner *ds, const char *filename);
};

class FAMDirScanner : public DirScanner {
public:
	FAMDirScanner(FAMHandler *_fh, bool _f = true) : DirScanner(_f), fh(_fh) { };
	FAMDirScanner(FAMHandler *_fh, const string& _p) : DirScanner(_p), fh(_fh) { };
	~FAMDirScanner() { if(! fake) fh->remove(this); }
	
	DirEntry* new_folder(bool fake) { return new FAMDirScanner(fh, fake); }
	DirEntry* new_folder(const string& path) { return new FAMDirScanner(fh, path); }
	
	FAMHandler *fh;
	FAMRequest famRequest;
	
	bool isFake() { return fake; }
};

FAMHandler::FAMHandler(const string& config_file, bool doBuddy, bool doReload)
           : save_at(0), root(0)
{
	FAMCONNECTION_GETFD(&fc) = -1;

	if (Scanner_Verbosity >= 2){
	    NNLOG.logEvent.connect(new NewNet::ConsoleOutput);
    	NNLOG.enable("ALL");
    }
 
	m_doReload = doReload;
   
	Muconf config(config_file);
	if(! config.hasDomain("shares") || ! config["shares"].hasKey("database")) {
		cerr << "config file '" << config_file << "' incomplete or corrupt shares" << endl;
		exit(-1);
	}
	if(! config.hasDomain("buddy.shares") || ! config["buddy.shares"].hasKey("database")) {
		cerr << "config file '" << config_file << "' incomplete or corrupt buddy shares" << endl;
		exit(-1);
	}
	if (doBuddy)
		{
		string tmp = config["buddy.shares"]["database"];
		shares = tmp;
		state = shares + ".state";
	}
	else {
		string tmp = config["shares"]["database"];
		shares = tmp;
		state = shares + ".state";
	}

}

FAMHandler::~FAMHandler()
{
	if(root)
		delete root;

	if(FAMCONNECTION_GETFD(&fc) != -1)
		FAMClose(&fc);
}

int
FAMHandler::load()
{
	if(root)
		delete root;

	if(FAMCONNECTION_GETFD(&fc) != -1)
		FAMClose(&fc);
	
	if(FAMOpen2(&fc, "muscand") < 0)
	{
		cerr << "couldn't connect to FAM" << endl;
		return -1;
	}
	
	root = new FAMDirScanner(this);
	root->load(state);
	add(root);

	return 0;
}

void
FAMHandler::add(FAMDirScanner *h)
{
	if(find(nodes.begin(), nodes.end(), h) != nodes.end())
		cerr << "double insertion" << endl;
	else if(h->isFake())
		cerr << "it's fake.." << endl;
	else
	{
		if(! h->path.empty())
		{
			cerr << "Registering " << h->path << endl;
			pending.push_back(h);
		}
	}
	
	map<string,DirEntry*>::iterator it = h->folders.begin(), end = h->folders.end();
	for(; it != end; ++it)
		add((FAMDirScanner*)(*it).second);
}

void
FAMHandler::remove(FAMDirScanner *h)
{
	if(h->isFake())
	{
		cerr << "trying to remove a fake entry" << endl;
		return;
	}
	
	vector<FAMDirScanner*>::iterator it = find(nodes.begin(), nodes.end(), h);
	if(it == nodes.end())
		cerr << "Possible corruption (removal of non-existing entry)" << endl;
	else
	{
		FAMCancelMonitor(&fc, &h->famRequest);
		nodes.erase(it);
	}
	
	vector<FAMDirScanner *>::iterator it2 = find(pending.begin(), pending.end(), h);
	if(it2 != pending.end())
	{
		cerr << "Removing pending registration" << endl;
		pending.erase(it2);
	}
}

int
FAMHandler::run()
{
	int fam_fd = FAMCONNECTION_GETFD(&fc);
	
	while(1)
	{
		if(save_at && time(NULL) >= save_at)
		{
			cerr << "saving updated shares database" << endl;
			save();
			save_at = 0;
		}
		
		fd_set rfds, wfds;
		struct timeval tv;
		
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		FD_SET(fam_fd, &rfds);
		if(! pending.empty())
			FD_SET(fam_fd, &wfds);
		
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		
		int retval = select(fam_fd+1, &rfds, &wfds, 0, &tv);
		if(retval == -1)
		{
			FAMClose(&fc);
			cerr << "Select error on FAM socket" << endl;
			return -1;
		}
		if(retval == 0)
			continue;
		
		if(FD_ISSET(fam_fd, &wfds) && pending.size())
		{
			vector<FAMDirScanner *>::iterator it = pending.begin();
			FAMMonitorDirectory(&fc, (*it)->path.c_str(), &(*it)->famRequest, *it);
			nodes.push_back(*it);
			pending.erase(it);
		}
		
		if(FD_ISSET(fam_fd, &rfds))
		{
			while(FAMPending(&fc))
			{
				FAMEvent fe;
				if(FAMNextEvent(&fc, &fe) < 0)
				{
					FAMClose(&fc);
					cerr << "Read error on FAM socket" << endl;
					return -1;
				}
				
				if(fe.code == FAMExists || fe.code == FAMCreated || fe.code == FAMChanged) {
					exists((FAMDirScanner *)fe.userdata, fe.filename, fe.code == FAMChanged);
				} else if(fe.code == FAMDeleted) {
					deleted(static_cast<FAMDirScanner*>(fe.userdata), fe.filename);
				} else
					cout << "ping " << fe.code << fe.filename << endl;
			}
		}
	}
}

void
FAMHandler::save()
{
	root->save(state);

	DirEntry folded;

	root->fold(&folded);
	folded.save(shares);
#ifndef WIN32
	if (m_doReload)
		system("killall -HUP museekd");
#endif // WIN32
}

void
FAMHandler::exists(FAMDirScanner *ds, const char *filename, bool force_file)
{
	std::string path = ds->path + "/" + filename;
	
	if(ds->path == filename || find(nodes.begin(), nodes.end(), ds) == nodes.end())
		return;
	
	struct stat s;
	if(stat(path.c_str(), &s) == -1)
		return;
	
	if(S_ISREG(s.st_mode))
	{
		if(force_file || ds->files.find(filename) == ds->files.end())
		{
			cout << "new file: " << path << endl;
			FileEntry fe = ds->scan_file(path);
			fe.size = s.st_size;
			ds->files[filename] = fe;
			save_at = time(0) + 15;
		}
	}
	else if(S_ISDIR(s.st_mode))
	{
		if(ds->folders.find(path) == ds->folders.end())
		{
			cout << "scanning: " << path << endl;
			ds->add(path);
			FAMDirScanner *ns = static_cast<FAMDirScanner *>(ds->folders[path]);
			ns->scan();
			add(ns);
			save_at = time(NULL) + 15;
		}
	}
}

void
FAMHandler::deleted(FAMDirScanner *ds, const char *filename)
{
	if(ds->path == filename)
	{
		cout << "hard deleting " << ds->path << " -- " << filename << endl;
		int ix = ds->path.rfind('/');
		string parent = ds->path.substr(0, ix), child = ds->path.substr(ix + 1);
		vector<FAMDirScanner *>::iterator it = nodes.begin(), end = nodes.end();
		for(; it != end; ++it)
		{
			if((*it)->path == parent)
			{
				cout << "found it" << endl;
				(*it)->folders.erase(filename);
				delete ds;
				save_at = time(0) + 15;
				return;
			}
		}
		cout << parent << " -- " << child << endl;
		return;
	}
	else
		ds->files.erase(filename);
}

void help() {
	cout << "muscand [-c --config PATH] [-b --buddy] [-h --help] [-v --verbose] [--no-reload]" << endl;
	cout << "Version 0.2.0" << endl;
	exit(-1);
}

int main(int argc, char **argv)
{
#ifdef RELAYED_LIBFAM
	extern int libfam_is_present;
	if(! libfam_is_present)
	{
		cerr << "libfam not found, aborting" << endl;
		return -1;
	}
#endif
	string config_file = string(getenv("HOME")) + "/.museekd/config.xml";
	bool doBuddy = false;
	bool doReload = true;
	for(int i = 1; i < argc; i++) {
		string arg = argv[i];
		if(arg == "-c" || arg == "--config") {
			++i;
			config_file = string(argv[i]);
		} 
		else if(arg == "-b" || arg == "--buddy") {
			++i;
			doBuddy = true;
		} 
		else if(arg == "-h" || arg == "--help") {
			++i;
			help();	
		}
		else if(arg == "-v" || arg == "--verbose") {
			Scanner_Verbosity += 1;
		}
		else if(arg == "--no-reload") {
			doReload = false;
		}
	}
	
	FAMHandler fh(config_file, doBuddy, doReload);
	if(fh.load())
		return -1;
	fh.run();
	
	return 0;
}
