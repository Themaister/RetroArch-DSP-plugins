#include "inireader.h"

CIniReader::CIniReader(const char*)
{}

int CIniReader::ReadInteger(const char*, const char*, int def)
{
   return def;
}

float CIniReader::ReadFloat(const char*, const char*, float def)
{
   return def;
}

bool CIniReader::ReadBoolean(const char*, const char*, bool def)
{
   return def;
}

const char* CIniReader::ReadString(const char*, const char*, const char* def)
{
   return def;
}
