#include "inireader.h"
#include <string>

CIniReader::CIniReader(const char* szFileName)
{
   conf = config_file_new(szFileName);
}

CIniReader::~CIniReader()
{
   if (conf)
      config_file_free(conf);
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

