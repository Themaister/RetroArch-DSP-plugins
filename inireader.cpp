#include "inireader.h"
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

ConfigFile::ConfigFile(const std::string &filename)
{
   auto base = GetBaseDir();
   base += "/";
#ifndef _WIN32
   base += "."; // Hidden file in $HOME
#endif
   base += filename;
   conf = config_file_new(base.c_str());
   if (!conf)
      conf = config_file_new(nullptr);
}

ConfigFile::~ConfigFile()
{
   if (conf)
      config_file_free(conf);
}

std::string ConfigFile::GetBaseDir()
{
#ifdef _WIN32
   char buf[MAX_PATH];
   GetCurrentDirectory(sizeof(buf), buf);
   return buf;
#else
   const char *home = getenv("HOME");
   if (home)
      return home;
   else
      return "/etc";
#endif
}

int ConfigFile::get_int(const std::string &key, int def_val)
{
   int res;
   if (config_get_int(conf, key.c_str(), &res))
      return res;
   else
      return def_val;
}

double ConfigFile::get_double(const std::string &key, double def_val)
{
   double res;
   if (config_get_double(conf, key.c_str(), &res))
      return res;
   else
      return def_val;
}

bool ConfigFile::get_bool(const std::string &key, bool def_val)
{
   bool res;
   if (config_get_bool(conf, key.c_str(), &res))
      return res;
   else
      return def_val;
}

std::string ConfigFile::get_string(const std::string &key, const std::string &def_val)
{
   char *str;
   if (config_get_string(conf, key.c_str(), &str))
   {
      std::string ret = str;
      free(str);
      return ret;
   }
   else
      return def_val;
}

ConfigFile& ConfigFile::set_string(const std::string &key, const std::string &val)
{
   config_set_string(conf, key.c_str(), val.c_str());
   return *this;
}

ConfigFile& ConfigFile::set_double(const std::string &key, double val)
{
   config_set_double(conf, key.c_str(), val);
   return *this;
}

ConfigFile& ConfigFile::set_int(const std::string &key, int val)
{
   config_set_int(conf, key.c_str(), val);
   return *this;
}

ConfigFile& ConfigFile::set_bool(const std::string &key, bool val)
{
   config_set_bool(conf, key.c_str(), val);
   return *this;
}

void ConfigFile::write(const std::string &filename)
{
   auto base = GetBaseDir();
   base += "/";
#ifndef _WIN32
   base += "."; // Hidden file in $HOME
#endif
   base += filename;

   config_file_write(conf, base.c_str());
}

