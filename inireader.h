#ifndef INIREADER_H
#define INIREADER_H

#include "config_file.h"

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
};

#endif//INIREADER_H
