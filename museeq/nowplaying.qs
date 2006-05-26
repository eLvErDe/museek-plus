/*

 Fields:

  %(file)        filename or URL
                 [xmms, amarok, mpd]
  
  %(artist)      artist
                 [amarok, mp3blaster, mpd]
  %(track)       name of the track
                 [amarok, mp3blaster, mpd]
  %(title)       %(artist) - %(track)
                 [xmms, amarok, mp3blaster, mpd]
  %(album)       album the track is on
                 [amarok, mp3blaster, mpd]
  %(trackno)     track number on the album
                 [mpd]
  %(tracks)      tracks on the album
                 []
  %(year)        year the track was released
                 [amarok]
  %(genre)       the genre the track belongs to
                 [amarok]
  
  %(pos)         position in the current track (m:ss)
                 [xmms, amarok, mpd]
  %(pos_sec)     position in the current track(seconds)
                 [amarok]
  %(pos_usec)    position in the current track (useconds)
                 [xmms]
  %(pos_%)       position in the current track (percentage)
                 [mpd]
  
  %(length)      length of the current track (m:ss)
                 [xmms, amarok]
  %(length_sec)  length of the current track (seconds)
                 [amarok, mp3blaster]
  %(length_usec) length of the current track (useconds)
                 [xmms]
  
  %(bitrate)     bitrate
                 [xmms, amarok, mp3blaster]
  %(samplerate)  samplerate
                 [xmms, mp3blaster]
  %(channels)    channels of the media file (number)
                 [xmms]
  
  %(status)      status of the player
                 [xmms, mp3blaster, mpd]
  
  %(pl_length)   length of the playlist
                 [xmms, mpd]
  %(pl_current)  current position in playlist
                 [xmms, mpd]

*/
  
  
var nowPlayingString = "/me is now listening to: %(title)";
var nowPlayingInsertString = "%(title)";
var nowPlayingCustomString = "/me is now listening to: %(title)";
var nowPlayingPlayer = "xmms";
var nowPlayingInfoPipe = "/tmp/xmms-info";
var nowPlayingAmarok = "amarok";
var nowPlayingMP3Blaster = "~/.mp3blaster-status";

function nowPlayingExpandHome(p)
{
	if(p.substring(0, 1) == "~")
		return Dir.home + "/" + p.substring(1);
	else
		return p;
}

function nowPlayingBuildXMMS(format)
{
	var title = "";
	var status = "";
	var tunes = "";
	var current = "";
	var usecpos = "";
	var pos = "";
	var bitrate = "";
	var samplefreq = "";
	var usectime = "";
	var time = "";
	var channels = "";
	var file = "";
	
	var infile = new File(nowPlayingExpandHome(nowPlayingInfoPipe));
	infile.open(File.ReadOnly);
	while (!infile.eof) {
		var line = infile.readLine();
		if(line == "")
			continue;
		line = line.substring(0, line.length - 1);
		if(line.startsWith("Title: "))
			title = line.substring(7);
		else if(line.startsWith("Status: "))
			status = line.substring(8);
		else if(line.startsWith("Tunes in playlist: "))
			tunes = line.substring(19);
		else if(line.startsWith("Currently playing: "))
			current = line.substring(19);
		else if(line.startsWith("uSecPosition: "))
			usecpos = line.substring(14);
		else if(line.startsWith("Position: "))
			pos = line.substring(10);
		else if(line.startsWith("uSecTime: "))
			usectime = line.substring(10);
		else if(line.startsWith("Time: "))
			time = line.substring(6);
		else if(line.startsWith("Current bitrate: "))
			bitrate = line.substring(17);
		else if(line.startsWith("Samping Frequency: "))
			samplefreq = line.substring(19);
		else if(line.startsWith("Sampling Frequency: "))
			samplefreq = line.substring(20);
		else if(line.startsWith("Channels: "))
			channels = line.substring(10);
		else if(line.startsWith("File: "))
			file = line.substring(6);
	}
	
	var s = format.replace("%(title)", title);
	s = s.replace("%(file)", file);
	s = s.replace("%(status)", status);
	s = s.replace("%(pl_length)", tunes);
	s = s.replace("%(pl_current)", current);
	s = s.replace("%(pos_usec)", usecpos);
	s = s.replace("%(pos)", pos);
	s = s.replace("%(length_usec)", usectime);
	s = s.replace("%(length)", time);
	s = s.replace("%(bitrate)", bitrate);
	s = s.replace("%(samplerate)", samplefreq);
	s = s.replace("%(channels)", channels);
	
	return s;
}

function nowPlayingGetAmarok(field)
{
	Process.execute([ 'dcop', nowPlayingAmarok, 'player', field ], '');
	return Process.stdout.substring(0, Process.stdout.length - 1);
}

function nowPlayingBuildAmarok(format)
{
	var s = format;
	
	if(s.find("%(title)") >= 0)
		s = s.replace("%(title)", nowPlayingGetAmarok("nowPlaying"));
	
	if(s.find("%(pos)") >= 0)
		s = s.replace("%(pos)", nowPlayingGetAmarok("currentTime"));
	
	if(s.find("%(pos_sec") >= 0)
		s = s.replace("%(pos_sec)", nowPlayingGetAmarok("trackCurrentTime"));
	
	if(s.find("%(length)") >= 0)
		s = s.replace("%(length)", nowPlayingGetAmarok("totalTime"));
	
	if(s.find("%(length_sec") >= 0)
		s = s.replace("%(length_sec)", nowPlayingGetAmarok("trackTotalTime"));
	
	if(s.find("%(bitrate)") >= 0)
		s = s.replace("%(bitrate)", nowPlayingGetAmarok("bitrate"));
	
	if(s.find("%(album)") >= 0)
		s = s.replace("%(album)", nowPlayingGetAmarok("album"));
	
	if(s.find("%(file)") >= 0)
		s = s.replace("%(file)", nowPlayingGetAmarok("encodedURL"));
	
	if(s.find("%(year)") >= 0)
		s = s.replace("%(year)", nowPlayingGetAmarok("year"));
	
	if(s.find("%(genre)") >= 0)
		s = s.replace("%(genre)", nowPlayingGetAmarok("genre"));
	
	if(s.find("%(status)") >= 0)
	{
		status = nowPlayingGetAmarok("status");
		if(status == "0")
			s = s.replace("%(status)", "stopped");
		else if(status == "1")
			s = s.replace("%(status)", "paused");
		else if(status == "2")
			s = s.replace("%(status)", "playing");
		else
			s = s.replace("%(status)", "unknown");
	}
	
	return s;
}

function nowPlayingBuildMPD(format)
{
	format = format.replace ("%(artist)" , "%artist%");
	format = format.replace ("%(track)"  , "%title%");
	format = format.replace ("%(title)"  , "%artist% - %title%");
	format = format.replace ("%(album)"  , "%album%");
	format = format.replace ("%(trackno)", "%track%");
	format = format.replace ("%(file)"   , "%file%");
	
	Process.execute ([ 'mpc', '--format', format ], '');
	var a = Process.stdout.split ("\n");
	var b = a[1].split (" ");
	var status = b[0].substring (1, b[0].length - 1);
	var c = b[1].substring (1, b[1].length).split ("/");
	var pl_current = c[0];
	var pl_length  = c[1];
	var pos_pctg   = b[5].substring (1, b[5].length - 1);
	var pos        = b[4];
	
	a = a[0].replace ("%(status)" , status);
	a = a.replace ("%(pos)"       , pos);
	a = a.replace ("%(pl_length)" , pl_length);
	a = a.replace ("%(pl_current)", pl_current);
	a = a.replace ("%(pos_%)"     , pos_pctg);
	
  	return a;
}

function nowPlayingBuildMP3Blaster(format)
{
	var file = "";
	var status = "";
	var artist = "";
	var album = "";
	var title = "";
	var bitrate = "";
	var samplerate = "";
	var length = "";
	
	var infile = new File(nowPlayingExpandHome(nowPlayingMP3Blaster));
	infile.open(File.ReadOnly);
	while (!infile.eof) {
		var line = infile.readLine();
		if(line == "")
			continue;
		line = line.substring(0, line.length - 1);
		if(line.startsWith("path "))
			file = line.substring(5);
		else if(line.startsWith("status "))
			status = line.substring(7);
		else if(line.startsWith("title "))
			title = line.substring(6);
		else if(line.startsWith("bitrate "))
			bitrate = line.substring(8);
		else if(line.startsWith("samplerate "))
			samplerate = line.substring(11);
		else if(line.startsWith("artist "))
			artist = line.substring(7);
		else if(line.startsWith("album "))
			album = line.substring(6);
		else if(line.startsWith("length "))
			length = line.substring(7);
	}
	
	var s = format.replace("%(title)", artist + " - " + title);
	s = s.replace("%(artist)", artist);
	s = s.replace("%(album)", album);
	s = s.replace("%(track)", title);
	s = s.replace("%(file)", file);
	s = s.replace("%(status)", status);
	s = s.replace("%(bitrate)", bitrate);
	s = s.replace("%(length_sec)", length);
	return s.replace("%(samplerate)", samplerate);
}

function nowPlayingBuild(format)
{
	if(nowPlayingPlayer == "xmms")
		return nowPlayingBuildXMMS(format);
	else if(nowPlayingPlayer == "amarok")
		return nowPlayingBuildAmarok(format);
	else if(nowPlayingPlayer == "mpd")
		return nowPlayingBuildMPD(format);
	else if(nowPlayingPlayer == "mp3blaster")
		return nowPlayingBuildMP3Blaster(format);
	
	return "";
}

function nowPlayingCheck()
{
	if(! Museeq.connected)
	{
		MessageBox.warning("You need to be connected to museekd to do this", MessageBox.Ok, MessageBox.NoButton);
		return 0;
	}
	
	if(! mainWin.chatRooms.current)
	{
		MessageBox.warning("You must select a chatroom to spew this in", MessageBox.Ok, MessageBox.NoButton);
		return 0;
	}
	
	return 1;
}

function nowPlaying()
{
	if(nowPlayingCheck() == 0)
		return;
	
	sayRoom(mainWin.chatRooms.current, nowPlayingBuild(nowPlayingString));
}

function nowPlayingInsert()
{
	if(nowPlayingCheck() == 0)
		return;
	
	var string = Input.getText("Text:", nowPlayingInsertString, "Insert now playing");
	if(string) {
		Museeq.setConfig("QSAnowPlaying", "insertString", string);
		mainWin.chatRooms.getCurrentWidget().panel.line.insert(nowPlayingBuild(string));
	}
}

function nowPlayingCustom()
{
	if(nowPlayingCheck() == 0)
		return;
	
	var string = Input.getText("Text:", nowPlayingCustomString, "Send custom now playing text");
	if(string) {
		Museeq.setConfig("QSAnowPlaying", "customString", string);
		sayRoom(mainWin.chatRooms.current, nowPlayingBuild(string));
	}
}

function nowPlayingConfigChanged(domain, key, value)
{
	if(domain == "QSAnowPlaying")
	{
		if(key == "string")
			nowPlayingString = value;
		else if(key == "insertString")
			nowPlayingInsertString = value;
		else if(key == "customString")
			nowPlayingCustomString = value;
		else if(key == "player")
			nowPlayingPlayer = value;
		else if(key == "infopipe")
			nowPlayingInfoPipe = value;
		else if(key == "amarok")
			nowPlayingAmarok = value;
		else if(key == "mp3blaster")
			nowPlayingMP3Blaster = value;
	}
}

function nowPlayingConfigure()
{
	if(! Museeq.connected)
	{
		MessageBox.warning("You need to be connected to museekd to do this", MessageBox.Ok, MessageBox.NoButton);
		return;
	}
	
	var dialog = new Dialog;
	dialog.title = "Configure Now Playing";
	
	var text = new LineEdit;
	text.label = "Default np text:";
	text.text = nowPlayingString;
	dialog.add(text);
	
	var group = new GroupBox;
	group.title = "Player";
	
	var xmms = new RadioButton;
	xmms.text = "XMMS / BMP (using InfoPipe)";
	if(nowPlayingPlayer == "xmms")
		xmms.checked = true;
	group.add(xmms);
	
	var infopipe = new LineEdit;
	infopipe.label = "Path to infopipe:";
	infopipe.text = nowPlayingInfoPipe;
	group.add(infopipe);
	
	var amarok = new RadioButton;
	amarok.text = "Amarok (using DCOP)";
	if(nowPlayingPlayer == "amarok")
		amarok.checked = true;
	group.add(amarok);
	
	var amarok_app = new LineEdit;
	amarok_app.label = "Amarok DCOP name:";
	amarok_app.text = nowPlayingAmarok;
	group.add(amarok_app);
	
	var mpd = new RadioButton;
	mpd.text = "MPD (using MPC)";
	if(nowPlayingPlayer == "mpd")
		mpd.checked = true;
	group.add(mpd);
	
	var mp3blaster = new RadioButton;
	mp3blaster.text = "MP3Blaster (using status file)";
	if(nowPlayingPlayer == "mp3blaster")
		mp3blaster.checked = true;
	group.add(mp3blaster);
	
	var mp3blaster_path = new LineEdit;
	mp3blaster_path.label = "Path to MP3Blaster status file:";
	mp3blaster_path.text = nowPlayingMP3Blaster;
	group.add(mp3blaster_path);
	
	dialog.add(group);
	
	if(dialog.exec())
	{
		Museeq.setConfig("QSAnowPlaying", "string", text.text);
		if(xmms.checked)
			Museeq.setConfig("QSAnowPlaying", "player", "xmms");
		else if(amarok.checked)
			Museeq.setConfig("QSAnowPlaying", "player", "amarok");
		else if(mpd.checked)
			Museeq.setConfig("QSAnowPlaying", "player", "mpd");
		else
			Museeq.setConfig("QSAnowPlaying", "player", "mp3blaster");
		
		Museeq.setConfig("QSAnowPlaying", "infopipe", infopipe.text);
		Museeq.setConfig("QSAnowPlaying", "amarok", amarok_app.text);
		Museeq.setConfig("QSAnowPlaying", "mp3blaster", mp3blaster_path.text);
	}
}

function nowPlayingInputHandler(privateMessage, target, line)
{
	if(line == "np")
		return nowPlayingBuild(nowPlayingString);
	else if(line.startsWith("np "))
		return nowPlayingBuild(line.substring(3));
}

function init()
{
	var value = config("QSAnowPlaying", "string");
	if(value)
		nowPlayingString = value;
	
	value = config("QSAnowPlaying", "insertString");
	if(value)
		nowPlayingInsertString = value;
	
	value = config("QSAnowPlaying", "customString");
	if(value)
		nowPlayingCustomString = value;
	
	value = config("QSAnowPlaying", "player");
	if(value)
		nowPlayingPlayer = value;
	
	value = config("QSAnowPlaying", "infopipe");
	if(value)
		nowPlayingInfoPipe = value;
	
	value = config("QSAnowPlaying", "amarok");
	if(value)
		nowPlayingAmarok = value;
	
	value = config("QSAnowPlaying", "mp3blaster");
	if(value)
		nowPlayingMP3Blaster = value;
	
	connect(Museeq, "configChanged(const QString&,const QString&,const QString&)", nowPlayingConfigChanged);
	addMenu("Scripts", "Send default now playing", "nowPlaying");
	addMenu("Scripts", "Send custom now playing", "nowPlayingCustom");
	addMenu("Scripts", "Insert now playing", "nowPlayingInsert");
	addMenu("Settings", "Configure now playing", "nowPlayingConfigure");
	
	addInputHandler("nowPlayingInputHandler");
	
	return "NowPlaying";
}
