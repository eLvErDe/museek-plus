/*  Museek - A SoulSeek client written in C++
    Copyright (C) 2006-2007 Ingmar K. Steen (iksteen@gmail.com)
    Copyright 2008 little blue poney <lbponey@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif // HAVE_CONFIG_H
#include "configmanager.h"
#include <NewNet/nnlog.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <cstring>

Museek::ConfigManager::ConfigManager() : m_AutoSave(true)
{
  /* Check if the libxml we linked against when ConfigManager was compiled
     is compatible with the libxml version we're linked against at runtime. */
  LIBXML_TEST_VERSION;
}

bool
Museek::ConfigManager::load(const std::string & path)
{
  NNLOG("museekd.config.debug", "Loading configuration '%s'.", path.c_str());

  // Load and parse the specified configuration file.
  xmlDoc * doc = xmlReadFile(path.c_str(), 0, 0);
  if(! doc)
  {
    NNLOG("museekd.config.warn", "Could not parse configuration file '%s'.", path.c_str());
    return false;
  }

  // Store the path so we can auto-save changes.
  m_Path = path;
  // While loading the configuration, disable autosave to prevent spurious saves.
  bool oldAutoSave = m_AutoSave;
  setAutoSave(false);

  // Get the root element.
  xmlNode * rootIter = xmlDocGetRootElement(doc);
  for(; rootIter; rootIter = rootIter->next)
  {
    // Check the root element (should be a <museekd> element).
    if((rootIter->type != XML_ELEMENT_NODE) || (strcmp((const char *)rootIter->name, "museekd") != 0))
      continue;
    // Iterate over the root element's children. Domains live there.
    for(xmlNode * domainIter = rootIter->children; domainIter; domainIter = domainIter->next)
    {
      // Domains are <domain> elements.
      if((domainIter->type != XML_ELEMENT_NODE) || (strcmp((const char *)domainIter->name, "domain") != 0))
        continue;

      // Get the domain id property.
      xmlChar * id = xmlGetProp(domainIter, (const xmlChar *)"id");
      if(! id)
      {
        NNLOG("museekd.config.warn", "Domain without id encountered in config file.");
        continue;
      }
      std::string domain((const char *)id);
      xmlFree(id);

      // Iterator over the domain element's children. Keys live there.
      for(xmlNode * keyIter = domainIter->children; keyIter; keyIter = keyIter->next)
      {
        // A key is a <key> element.
        if((keyIter->type != XML_ELEMENT_NODE) || (strcmp((const char *)keyIter->name, "key") != 0))
          continue;

        // Get the key's id property.
        xmlChar * id = xmlGetProp(keyIter, (const xmlChar *)"id");
        if(! id)
        {
          NNLOG("museekd.config.warn", "Key without id encountered in config file.");
          continue;
        }
        std::string key((const char *)id);
        xmlFree(id);

        // The data of the key is its value.
        xmlChar * data = xmlNodeListGetString(doc, keyIter->children, true);
        if(data)
        {
          set(domain, key, (const char *)data);
          xmlFree(data);
        }
        else
        {
          // No data, no value.
          set(domain, key, "");
        }
      }
    }
  }

  // Restore the auto-save state.
  setAutoSave(oldAutoSave);

  // Clean up xml things.
  xmlFreeDoc(doc);
  xmlCleanupParser();

  // See if we need to update the config file
  updateConfigFile();

  return true;
}

bool
Museek::ConfigManager::save(const std::string & path) const
{
  // Check if we know where to save the configuration.
  if(path.empty() && m_Path.empty())
  {
    NNLOG("museekd.config.warn", "No path to save configuration to specified.");
    return false;
  }

  NNLOG("museekd.config.config.debug", "Saving configuration to '%s'.", path.empty() ? m_Path.c_str() : path.c_str());

  // Build the xml document.
  xmlDocPtr doc = xmlNewDoc((const xmlChar *)"1.0");
  // Create and set the root node (<museekd>).
  xmlNodePtr rootNode = xmlNewNode(NULL, (const xmlChar *)"museekd");
  xmlDocSetRootElement(doc, rootNode);

  // Iterate over the domains.
  Config::const_iterator it, end = m_Config.end();
  for(it = m_Config.begin(); it != end; ++it)
  {
    // Create a <domain> element for this domain.
    xmlNodePtr domainNode = xmlNewChild(rootNode, NULL, (const xmlChar *)"domain", NULL);
    // Set the id property of the domain element.
    xmlNewProp(domainNode, (const xmlChar *)"id", (const xmlChar *)(*it).first.c_str());
    // Iterate over the domain's keys.
    Domain::const_iterator it2, end2 = (*it).second.end();
    for(it2 = (*it).second.begin(); it2 != end2; ++it2)
    {
      xmlNodePtr keyNode;
      /* If the key has a value, create a new key node with the appropriate
         value. */
      if(! (*it2).second.empty())
        keyNode = xmlNewChild(domainNode, NULL, (const xmlChar *)"key", (const xmlChar *)(*it2).second.c_str());
      else
        keyNode = xmlNewChild(domainNode, NULL, (const xmlChar *)"key", NULL);
      // Set the key's id property.
      xmlNewProp(keyNode, (const xmlChar *)"id", (const xmlChar *)(*it2).first.c_str());
    }
  }

  // Save the xml document.
  xmlSaveFormatFileEnc(path.empty() ? m_Path.c_str() : path.c_str(), doc, "UTF-8", 1);

  // Clean up.
  xmlFreeDoc(doc);
  xmlCleanupParser();

  return true;
}

/**
  * Update config.xml
  */
void
Museek::ConfigManager::updateConfigFile() {
    std::string version = get("museekd", "version", "none");

    if (version == "none") {
        // Probably updating from 0.1.13 or previous

        // Switching to the new server
        std::string host = get("server", "host");
        uint port = getUint("server", "port");
        if ((host == "server.slsknet.org") && (port != 2242))
            set("server", "port", 2242);
    }
    else if (version == "0.2.0") {
        // Nothing special to do.
        // One new config key: "priv_rooms"/"enable_priv_room" -> false if not present
    }
    // Setting the new version
    set("museekd", "version", "0.3.0");
}

std::string
Museek::ConfigManager::get(const std::string & domain, const std::string & key, const std::string & defaultValue) const
{
  // Try to find the domain.
  Config::const_iterator it = m_Config.find(domain);
  if(it == m_Config.end())
    return defaultValue; // Domain not found, return default value.
  // Try to find the key.
  Domain::const_iterator it2 = (*it).second.find(key);
  if(it2 == (*it).second.end())
    return defaultValue; // Key not found, return default value.
  // Return the key's value.
  return (*it2).second;
}

unsigned int
Museek::ConfigManager::getUint(const std::string & domain, const std::string & key, unsigned int defaultValue) const
{
  // Get the string value from the configuration.
  std::string value(get(domain, key));
  if(value.empty())
    return defaultValue; // No such value, return the default value.
  return atol(value.c_str()); // Convert the string to an unsigned integer value.
}

int
Museek::ConfigManager::getInt(const std::string & domain, const std::string & key, int defaultValue) const
{
  // Get the string value from the configuration.
  std::string value(get(domain, key));
  if(value.empty())
    return defaultValue; // No such value, return the default value.
  return atol(value.c_str()); // Convert the string to a signed integer value.
}

double
Museek::ConfigManager::getDouble(const std::string & domain, const std::string & key, double defaultValue) const
{
  // Get the string value from the configuration.
  std::string value(get(domain, key));
  if(value.empty())
    return defaultValue; // No such value, return the default value.
  return strtod(value.c_str(), 0); // Convert the string to a double value.
}

bool
Museek::ConfigManager::getBool(const std::string & domain, const std::string & key, bool defaultValue) const
{
  // Get the string value from the configuration.
  std::string value(get(domain, key));
  if(value.empty())
    return defaultValue; // No such value, return the default value.
  return value == "true"; // If the value is 'true' return true, false otherwise.
}

void
Museek::ConfigManager::set(const std::string & domain, const std::string & key, const std::string & value)
{
  // Set the value in the configuration.
  m_Config[domain][key] = value;
  // Save the configuration if auto-save is enabled.
  doAutoSave();
  // Emit the key set / changed event.
  emitKeySetEvent(domain, key, value);
}

void
Museek::ConfigManager::set(const std::string & domain, const std::string & key, const char * value)
{
  // Set the value in the configuration.
  m_Config[domain][key] = value;
  // Save the configuration if auto-save is enabled.
  doAutoSave();
  // Emit the key set / changed event.
  emitKeySetEvent(domain, key, value);
}

void
Museek::ConfigManager::set(const std::string & domain, const std::string & key, unsigned int value)
{
  // Format the unsigned integer to a string value.
  char x[80];
  snprintf(x, 80, "%u", value);
  // Set the value in the configuration.
  m_Config[domain][key] = x;
  // Save the configuration if auto-save is enabled.
  doAutoSave();
  // Emit the key set / changed event.
  emitKeySetEvent(domain, key, x);
}

void
Museek::ConfigManager::set(const std::string & domain, const std::string & key, int value)
{
  // Format the unsigned integer to a string value.
  char x[80];
  snprintf(x, 80, "%i", value);
  // Set the value in the configuration.
  m_Config[domain][key] = x;
  // Save the configuration if auto-save is enabled.
  doAutoSave();
  // Emit the key set / changed event.
  emitKeySetEvent(domain, key, x);
}

void
Museek::ConfigManager::set(const std::string & domain, const std::string & key, double value)
{
  // Format the unsigned integer to a string value.
  char x[80];
  snprintf(x, 80, "%f", value);
  // Set the value in the configuration.
  m_Config[domain][key] = x;
  // Save the configuration if auto-save is enabled.
  doAutoSave();
  // Emit the key set / changed event.
  emitKeySetEvent(domain, key, x);
}

void
Museek::ConfigManager::set(const std::string & domain, const std::string & key, bool value)
{
  // Set the value in the configuration.
  m_Config[domain][key] = value ? "true" : "false";
  // Save the configuration if auto-save is enabled.
  doAutoSave();
  // Emit the key set / changed event.
  emitKeySetEvent(domain, key, value ? "true" : "false");
}

void
Museek::ConfigManager::removeKey(const std::string & domain, const std::string & key)
{
  // Check if the domain exists.
  Config::iterator it = m_Config.find(domain);
  if(it == m_Config.end())
    return; // Nope, nothing to do.
  // Check if the key exists in the domain.
  Domain::iterator it2 = (*it).second.find(key);
  if(it2 == (*it).second.end())
    return; // Nope, nothing to do.
  // Remove the key (and its value) from the configuration.
  (*it).second.erase(it2);

  // Save the configuration if auto-save is enabled.
  doAutoSave();

  // Prepare key-removed event data.
  RemoveNotify data;
  data.domain = domain;
  data.key = key;
  // Emit the key removed event.
  keyRemovedEvent(&data);
}

bool
Museek::ConfigManager::hasDomain(const std::string & domain) const
{
  // Check if the domain exists.
  return m_Config.find(domain) != m_Config.end();
}

bool
Museek::ConfigManager::hasKey(const std::string & domain, const std::string & key) const
{
  // Check if the domain exists.
  Config::const_iterator it = m_Config.find(domain);
  if(it == m_Config.end())
    return false; // Nope, key can't exist either.
  // Check if the key exists.
  return (*it).second.find(key) != (*it).second.end();
}

std::vector<std::string>
Museek::ConfigManager::keys(const std::string & domain) const
{
  std::vector<std::string> result;

  // Check if the domain exists.
  Config::const_iterator it(m_Config.find(domain));
  if(it == m_Config.end())
    return result; // Nope, no keys without a domain.

  // Iterate over the domain's keys.
  Domain::const_iterator it2, end2((*it).second.end());
  for(it2 = (*it).second.begin(); it2 != end2; ++it2)
    result.push_back((*it2).first); // And push them on the list.

  // Return the result.
  return result;
}

void
Museek::ConfigManager::emitKeySetEvent(const std::string & domain, const std::string & key, const std::string & value)
{
  // Set up the event's data object.
  ChangeNotify data;
  data.domain = domain;
  data.key = key;
  data.value = value;
  // Emit the key set event.
  keySetEvent(&data);
}
