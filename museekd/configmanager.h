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

#ifndef MUSEEK_CONFIGMANAGER_H
#define MUSEEK_CONFIGMANAGER_H

#include <NewNet/nnobject.h>
#include <NewNet/nnevent.h>
#include <string>
#include <map>

namespace Museek
{
  class ConfigManager : public NewNet::Object
  {
  public:
    /* A configuration domain contains keys and values. */
    typedef std::map<std::string, std::string> Domain;
    /* Configuration holds various domains. */
    typedef std::map<std::string, Domain> Config;

    /* A value was set or changed. */
    struct ChangeNotify
    {
      std::string domain, key, value;
    };

    /* A value was removed. */
    struct RemoveNotify
    {
      std::string domain, key;
    };

    ConfigManager();

    /* Load configuration from given path. */
    bool load(const std::string & path);
    /* Save configuration to given path (or overwrite existing configuration
       if path == std::string(). */
    bool save(const std::string & path = std::string()) const;

    /* Control wether changes are automatically saved. */
    void setAutoSave(bool autoSave)
    {
      m_AutoSave = autoSave;
    }

    /* Get a string from the configuration. */
    std::string get(const std::string & domain, const std::string & key, const std::string & defaultValue = std::string()) const;
    /* Get an unsigned integer from the configuration. */
    unsigned int getUint(const std::string & domain, const std::string & key, unsigned int defaultValue = 0) const;
    /* Get a signed integer from the configuration. */
    int getInt(const std::string & domain, const std::string & key, int defaultValue = 0) const;
    /* Get a double value from the configuration. */
    double getDouble(const std::string & domain, const std::string & key, double defaultValue = 0.0) const;
    /* Get a boolean value from the configuration. */
    bool getBool(const std::string & domain, const std::string & key, bool defaultValue = false) const;

    /* Set or change a string value in the configuration. */
    void set(const std::string & domain, const std::string & key, const std::string & value);
    /* Set or change a string value in the configuration. */
    void set(const std::string & domain, const std::string & key, const char * value);
    /* Set or change an unsigned integer in the configuration. */
    void set(const std::string & domain, const std::string & key, unsigned int value);
    /* Set or change a signed integer in the configuration. */
    void set(const std::string & domain, const std::string & key, int value);
    /* Set or change a double value in the configuration. */
    void set(const std::string & domain, const std::string & key, double value);
    /* Set or change a boolean in the configuration. */
    void set(const std::string & domain, const std::string & key, bool value);

    /* Remove a key (and its value) from the configuration. */
    void removeKey(const std::string & domain, const std::string & key);

    /* Check if a domain exists in the configuration. */
    bool hasDomain(const std::string & domain) const;
    /* Check if a key exists in the specified domain. */
    bool hasKey(const std::string & domain, const std::string & key) const;

    /* Get an immutable reference to the configuration data. */
    const Config & data() const
    {
      return m_Config;
    }

    /* Get an immutable reference to the domain data. */
    const Domain & domain(const std::string & domain)
    {
      return m_Config[domain];
    }

    /* Get a list of keys that exist in the specified domain. */
    std::vector<std::string> keys(const std::string & domain) const;

    /* This event is emitted when a value in the configuration is set or
       changed. */
    NewNet::Event<const ChangeNotify *> keySetEvent;
    /* This event is emitted when a key (and its value) is removed from
       the configuration. */
    NewNet::Event<const RemoveNotify *> keyRemovedEvent;

  protected:
    /* Save configuration is auto-save is enabled. */
    void doAutoSave() const
    {
      if(m_AutoSave)
        save();
    }

  private:
    /* Helper function to easily emit a key set event. */
    void emitKeySetEvent(const std::string & key, const std::string & domain, const std::string & value);

    /* Update config.xml */
    void updateConfigFile();

    /* Wether auto-save is enabled. */
    bool m_AutoSave;
    /* Path to the last loaded configuration file. */
    std::string m_Path;
    /* The configuration data. */
    Config m_Config;
  };
}

#endif // MUSEEK_CONFIGMANAGER_H
