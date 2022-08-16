#pragma once
#include <map>
#include <string>
#include "MyThread.h"

class CCommandThread : public MyThread
{
private:
    typedef void (CCommandThread::*CmdFunc)(const char* params);
    typedef std::map<std::string, CmdFunc> CmdMap; 
    CmdMap m_cmdMap;
    void cmdStatus(const char* params);
    void cmdSet(const char* params);
public:
    CCommandThread(void);
    ~CCommandThread(void);
    void Execute();
};