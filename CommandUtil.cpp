#include "CommandUtil.h"
#include <Windows.h>

using namespace std;

vector<string> ParseCmdSingleParam(const char * s, int len)
{
	vector<string> list;
	int i = 0;
	while (i < len)
	{
		string r = "";
		while (i < len && s[i] <= 32) i ++;

		while (i < len && s[i] > 32)
		{
			r.append(1, s[i]);
			i ++;
		}
		if (r != "")
		{
			list.push_back(r);
			if (i < len - 1)
			{
				list.push_back(s + i + 1);
			}
			break;
		}
	}
	return list;
}

vector<string> ParseCmdMultiParam(const char * s, int len)
{
	vector<string> list;
	int i = 0;
	while (i < len)
	{
		string r = "";
		while (i < len && s[i] <= 32) i ++;
        // ×Ö·û´®
        if (s[i] == '"')
        {
            i++;
            while (s[i] != '"' && s[i] != '\0')
            {
                r.append(1, s[i]);
                i++;
            }
            if (s[i] != '\0')
                i++;
            list.push_back(r);
        }
        else
        {
            while (i < len && s[i] > 32)
            {
                r.append(1, s[i]);
                i ++;
            }
            if (r != "")
            {
                list.push_back(r);
            }
        }
	}
	return list;
}


bool EqualNoCase(const char * s1, const char * s2)
{
	while (*s1 != 0 || *s2 != 0)
	{
		if (toupper(*s1) != toupper(*s2)) return false;
		s1++; s2++;
	}
	return true;
}

bool FileExists(const char * filename)
{
	bool r = false;
	WIN32_FIND_DATA FindFileData;
	HANDLE h = FindFirstFile(filename, &FindFileData);
	if (h == INVALID_HANDLE_VALUE)
	{
		return false;
	}else
	{
		do 
		{
			if (! (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			{			
				r = true;
				break;
			}
		}while (FindNextFile(h, &FindFileData));
			FindClose(h);
		return r;
	}
}

bool DirExists(const char * dirname)
{
	bool r = false;
	WIN32_FIND_DATA FindFileData;
	HANDLE h = FindFirstFile(dirname, &FindFileData);
	if (h == INVALID_HANDLE_VALUE)
	{
		return false;
	}else
	{
		do 
		{
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{			
				r = true;
				break;
			}
		}while (FindNextFile(h, &FindFileData));
		FindClose(h);
		return r;
	}
}
