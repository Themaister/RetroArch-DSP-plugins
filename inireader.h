#ifndef INIREADER_H
#define INIREADER_H

#include "config_file.h"
#include <string>

class CIniReader
{
   public:
      CIniReader (const char* szFileName); 
      ~CIniReader ();
      int ReadInteger (const char *szSection, const char* szKey, int iDefaultValue);
      float ReadFloat (const char *szSection, const char* szKey, float fltDefaultValue);
      bool ReadBoolean (const char *szSection, const char* szKey, bool bolDefaultValue);

   private:
      config_file_t *conf;
      static std::string GetBaseDir(const char *filename);
};

#endif//INIREADER_H
