#pragma once

#include <vector>
#include <string>

std::vector<std::string> ParseCmdMultiParam(const char * s, int len); 
std::vector<std::string> ParseCmdSingleParam(const char * s, int len); 
bool EqualNoCase(const char * s1, const char * s2);
bool FileExists(const char * filename);
bool DirExists(const char * dirname);
