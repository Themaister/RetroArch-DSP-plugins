#ifndef INIREADER_H
#define INIREADER_H

#include "config_file.h"
#include <string>

class ConfigFile
{
   public:
      ConfigFile(const std::string &filename); 
      ~ConfigFile();
      int get_int(const std::string &key, int def_val);
      double get_double(const std::string &key, double def_val);
      bool get_bool(const std::string &key, bool def_val);
      std::string get_string(const std::string &key, const std::string &def_val);

      ConfigFile& set_string(const std::string &key, const std::string &val);
      ConfigFile& set_double(const std::string &key, double val);
      ConfigFile& set_int(const std::string &key, int val);
      ConfigFile& set_bool(const std::string &key, bool val);

      void write(const std::string &filename);

   private:
      config_file_t *conf;
      static std::string GetBaseDir();
};

#endif

