/* Muhelp - Helper library for Museek
 *
 * Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
 * Copyright 2008 little blue poney <lbponey@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <system.h>

#include <Muhelp/Muconf.hh>

#include <libxml++/libxml++.h>
#include <Muhelp/string_ext.hh>
#include <NewNet/nnlog.h>

Muconf::Muconf() { };

Muconf::Muconf(const std::string& filename)
        : mFilename(filename) {
	NNLOG("museek.muconf", "Muconf %s", filename.c_str());
	
	try {
		xmlpp::DomParser parser;
		parser.set_substitute_entities();
		parser.parse_file(mFilename);
		if(parser) {
			xmlpp::Element* root = parser.get_document()->get_root_node();
			if(root->get_name() != "museekd") {
				NNLOG("museek.muconf", "Invalid configuration database, root element name mismatch");
				return;
			}
			restore(parser.get_document()->get_root_node());
		}
	} catch(const std::exception& ex) {
		NNLOG("museek.muconf", "Exception (%s) caught while parsing configuration database", ex.what());
	}
}

std::string Muconf::filename() const {
	return mFilename;
}

bool Muconf::hasDomain(const std::string& domain) const {
	NNLOG("museek.muconf", "hasDomain %s", domain.c_str());
	
	return mDomains.find(domain) != mDomains.end();
}

std::vector<std::string> Muconf::domains() const {
	NNLOG("museek.muconf", "domains");
	
	std::vector<std::string> d;
	
	std::map<std::string, MuconfDomain>::const_iterator it = mDomains.begin();
	for(; it != mDomains.end(); ++it)
		d.push_back((*it).first);
	
	return d;
}

MuconfDomain& Muconf::operator[](const std::string& domain) {
	NNLOG("museek.muconf", "operator[] %s", domain.c_str());
	
	std::map<std::string, MuconfDomain>::iterator it = mDomains.find(domain);
	if(it != mDomains.end())
		return (*it).second;
	
	mDomains[domain] = MuconfDomain(domain);
	
	return mDomains[domain];
}

void Muconf::restore(const xmlpp::Element* root) {
	NNLOG("museek.muconf", "restore <...>");
	
	const xmlpp::Node::NodeList nodes = root->get_children("domain");
	xmlpp::Node::NodeList::const_iterator it = nodes.begin();
	for(; it != nodes.end(); ++it) {
		const xmlpp::Element* elem = dynamic_cast<const xmlpp::Element*>(*it);
		if(! elem) {
			NNLOG("museek.muconf", "Domain node is not an element");
			continue;
		}
		xmlpp::Attribute* attr = elem->get_attribute("id");
		if(! attr) {
			NNLOG("museek.muconf", "Domain is missing id attribute");
			continue;
		}
		const std::string& domain = attr->get_value();
		if(mDomains.find(domain) == mDomains.end())
			mDomains[domain] = MuconfDomain(domain);
		mDomains[domain].restore(elem);
	}
}

void Muconf::store() {
	NNLOG("museek.muconf", "store");
	
	xmlpp::Document doc;
	xmlpp::Element* root = doc.create_root_node("museekd");
	std::map<std::string,MuconfDomain>::const_iterator it = mDomains.begin();
	for(; it != mDomains.end(); ++it) {
		root->add_child_text("\n  ");
		xmlpp::Element* node = root->add_child("domain");
		node->set_attribute("id", (*it).first);
		(*it).second.store(node);
		node->add_child_text("\n  ");
		
	}
	root->add_child_text("\n");
	try {
		doc.write_to_file(mFilename);
	} catch(const std::exception& ex) {
		NNLOG("museek.muconf", "Exception (%s) caught while writing configuration database to disk", ex.what());
	}
}

Muconf::operator std::map<std::string, std::map<std::string, std::string> >() const {
	NNLOG("museek.muconf", "operator std::map<std::string, std::map<std::string, std::string> >");
	
	std::map<std::string, std::map<std::string, std::string> > r;
	
	std::map<std::string, MuconfDomain>::const_iterator it = mDomains.begin();
	for(; it != mDomains.end(); ++it)
		r[(*it).first] = (*it).second;
	
	return r;
}	

#undef MULOG_DOMAIN
#define MULOG_DOMAIN "Muconf.ND"
	
MuconfDomain::MuconfDomain(const std::string& domain)
              : mDomain(domain) {
	NNLOG("museek.muconf", "MuconfDomain <...> %s", domain.c_str());
}

bool MuconfDomain::hasKey(const std::string& key) const {
	NNLOG("museek.muconf", "hasKey %s", key.c_str());
	
	return mKeys.find(key) != mKeys.end();
}

std::vector<std::string> MuconfDomain::keys() const {
	NNLOG("museek.muconf", "keys");
	
	std::vector<std::string> d;
	
	std::map<std::string, MuconfKey>::const_iterator it = mKeys.begin();
	for(; it != mKeys.end(); ++it)
		d.push_back((*it).first);
	
	return d;
}

MuconfKey& MuconfDomain::operator[](const std::string& key) {
	NNLOG("museek.muconf", "operator[] %s", key.c_str());
	
	std::map<std::string, MuconfKey>::iterator it = mKeys.find(key);
	if(it != mKeys.end())
		return (*it).second;
	
	mKeys[key] = MuconfKey(key);
	
	return mKeys[key];
}

void MuconfDomain::restore(const xmlpp::Element* root) {
	NNLOG("museek.muconf", "restore <...>");
	
	const xmlpp::Node::NodeList nodes = root->get_children("key");
	xmlpp::Node::NodeList::const_iterator it = nodes.begin();
	for(; it != nodes.end(); ++it) {
		const xmlpp::Element* elem = static_cast<const xmlpp::Element*>(*it);
		if(! elem) {
			NNLOG("museek.muconf", "Key node is not an element (domain: %s)", mDomain.c_str());
			continue;
		}
		xmlpp::Attribute* attr = elem->get_attribute("id");
		if(! attr) {
			NNLOG("museek.muconf", "Key is missing id attribute (domain %s)", mDomain.c_str());
			continue;
		}
		std::string key = attr->get_value();
		const xmlpp::TextNode* text = elem->get_child_text();
		if(mKeys.find(key) == mKeys.end())
			mKeys[key] = MuconfKey(key);
		if(text)
			mKeys[key] = text->get_content();
		else
			mKeys[key] = "";
	}	
}

void MuconfDomain::store(xmlpp::Element* root) const {
	NNLOG("museek.muconf", "store <...>");
	
	std::map<std::string, MuconfKey>::const_iterator it = mKeys.begin();
	for(; it != mKeys.end(); ++it) {
		if((*it).first == "")
			continue;
		const std::string& value = (*it).second;
		root->add_child_text("\n    ");
		xmlpp::Element* node = root->add_child("key");
		node->set_attribute("id", (*it).first);
		if(value != "")
			node->set_child_text(value);
	}
}

MuconfDomain::operator std::map<std::string, std::string>() const {
	NNLOG("museek.muconf", "operator std::map<std::string, std::string>");
	
	std::map<std::string, std::string> r;
	
	std::map<std::string, MuconfKey>::const_iterator it = mKeys.begin();
	for(; it != mKeys.end(); ++it)
		r[(*it).first] = (const char*)(*it).second;
	
	return r;
}

std::string MuconfDomain::domain() const {
	NNLOG("museek.muconf", "domain");
	
	return mDomain;
}

void MuconfDomain::remove(const std::string& key) {
	NNLOG("museek.muconf", "remove %s", key.c_str());
	
	mKeys.erase(key);
}

#undef MULOG_DOMAIN
#define MULOG_DOMAIN "Muconf.NK"

MuconfKey::MuconfKey(const std::string& key)
           : mKey(key) {
	NNLOG("museek.muconf", "MuconfKey::MuconfKey <...> %s", mKey.c_str());
}

void MuconfKey::operator=(const std::string& value) {
	NNLOG("museek.muconf", "MuconfKey::operator= %s", value.c_str());
	
	mValue = value;
}

void MuconfKey::operator=(const char* value) {
	NNLOG("museek.muconf", "MuconfKey::operator= %s", value);
	
	mValue = std::string(value);
}

void MuconfKey::operator=(uint value) {
	NNLOG("museek.muconf", "MuconfKey::operator= %u", value);
	
	char x[80];
	snprintf(x, 80, "%u", value);
	mValue = x;
}

void MuconfKey::operator=(int value) {
	NNLOG("museek.muconf", "MuconfKey::operator= %i", value);
	
	char x[80];
	snprintf(x, 80, "%i", value);
	mValue = x;
}

void MuconfKey::operator=(double value) {
	NNLOG("museek.muconf", "MuconfKey::operator= %f", value);
	
	char x[80];
	snprintf(x, 80, "%f", value);
	mValue = x;
}

void MuconfKey::operator=(bool value) {
	NNLOG("museek.muconf", "MuconfKey::operator= %d", value);
	if(value)
		mValue = "true";
	else
		mValue = "false";
}

MuconfKey::operator std::string() const {
	NNLOG("museek.muconf", "MuconfKey::operator std::string");
	
	return mValue;
}

MuconfKey::operator const char*() const {
	NNLOG("museek.muconf", "MuconfKey::operator const char*");
	
	return mValue.c_str();
}

uint MuconfKey::asUint() const {
	NNLOG("museek.muconf", "MuconfKey::asUint");
	
	return atol(mValue.c_str());
}

int MuconfKey::asInt() const {
	NNLOG("museek.muconf", "MuconfKey::asInt");

	return atoi(mValue.c_str());
}

double MuconfKey::asDouble() const {
	NNLOG("museek.muconf", "MuconfKey::asDouble");
	
	return strtod(mValue.c_str(), NULL);
}

bool MuconfKey::asBool() const {
	NNLOG("museek.muconf", "MuconfKey::asBool");
	
	if(tolower(mValue) == "true" || asInt() != 0)
		return true;
	else
		return false;
}

bool MuconfKey::operator==(const std::string& v) const {
	NNLOG("museek.muconf", "MuconfKey::operator== %s", v.c_str());
	
	return mValue == v;
}

bool MuconfKey::operator!=(const std::string& v) const {
	NNLOG("museek.muconf", "MuconfKey::operator!= %s", v.c_str());
	
	return mValue != v;
}

bool MuconfKey::operator!() const {
	return mValue.size() == 0;
}
