#include "inireader.h"
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

CIniReader::CIniReader(const char* szFileName)
{
   std::string base = GetBaseDir(szFileName);
   base += "/";
#ifndef _WIN32
   base += "."; // Hidden file in $HOME
#endif
   base += szFileName;
   conf = config_file_new(base.c_str());
   if (!conf)
      conf = config_file_new(NULL);
}

CIniReader::~CIniReader()
{
   if (conf)
      config_file_free(conf);
}

std::string CIniReader::GetBaseDir(const char *filename)
{
#ifdef _WIN32
   char buf[256];
   GetCurrentDirectory(sizeof(buf), buf);
   return buf;
#else
   (void)filename;
   const char *home = getenv("HOME");
   if (home)
      return home;
   else
      return "/etc";
#endif
}

int CIniReader::ReadInteger(const char* szSection, const char* szKey, int iDefaultValue)
{
   std::string key(szSection);
   key += "_";
   key += szKey;
   int res;
   if (config_get_int(conf, key.c_str(), &res))
      return res;
   else
      return iDefaultValue;
}

float CIniReader::ReadFloat(const char* szSection, const char* szKey, float fltDefaultValue)
{
   std::string key(szSection);
   key += "_";
   key += szKey;
   double res;
   if (config_get_double(conf, key.c_str(), &res))
      return (float)res;
   else
      return fltDefaultValue;
}

bool CIniReader::ReadBoolean(const char* szSection, const char* szKey, bool bolDefaultValue)
{
   std::string key(szSection);
   key += "_";
   key += szKey;
   bool res;
   if (config_get_bool(conf, key.c_str(), &res))
      return res;
   else
      return bolDefaultValue;
}

std::string CIniReader::ReadString(const char *szSection, const char *szKey, const char *szDefaultValue)
{
   std::string key(szSection);
   key += "_";
   key += szKey;
   char *str;
   if (config_get_string(conf, key.c_str(), &str))
   {
      std::string ret = str;
      free(str);
      return ret;
   }
   else
      return szDefaultValue;
}
