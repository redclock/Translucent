#include <iostream>
#include "CommandThread.h"
#include "CommandUtil.h"
#include "MyGlobal.h"

using namespace std;

CCommandThread::CCommandThread(void) : MyThread()
{
    m_cmdMap["?"] = &CCommandThread::cmdStatus;
    m_cmdMap["set"] = &CCommandThread::cmdSet;
}

CCommandThread::~CCommandThread()
{

}

void CCommandThread::Execute()
{
    while (! m_terminating)
    {
        char cmd[256];
        cout << "]";
        cin.getline(cmd, 255);
        vector<string> listCmd = ParseCmdSingleParam(cmd, (int) strlen(cmd));
        if (listCmd.size() > 0)
        {
            
            CmdMap::iterator itr = m_cmdMap.find(listCmd[0]);
            if (itr != m_cmdMap.end())
            {
                CmdFunc func = (itr->second);
                if (listCmd.size() > 1)
                    (this->*func)(listCmd[1].c_str());
                else
                    (this->*func)("");
            }
            else
            {
                cout << "�������\n";
            }
        }
        cout << endl;
    }
}

void CCommandThread::cmdStatus( const char* params )
{
    vector<string> listCmd = ParseCmdSingleParam(params, (int)strlen(params));
    if (listCmd.size() == 0 || listCmd[0] == "option")
    {
        cout << "    ��������   : " << g_RenderOption.sampleCount << endl;
        cout << "    �������   : " << g_RenderOption.maxErr << endl;
        cout << "    ��͸������ : " << g_RenderOption.transFactor << endl;
    }
    if (listCmd.size() == 0 || listCmd[0] == "state")
    {
        cout << "    ��ǰ״̬   : ";
        if (g_StateInfo.isPreview) 
            cout << "Ԥ��ģ��";
        else
            cout << "��Ⱦ���ս��";
        cout << endl;
        cout << "    ģ���ļ�   : " << g_StateInfo.modelFile << endl;
    }
    if (listCmd.size() == 0 || listCmd[0] == "bssrdf")
    {
        cout << "    ɢ��ϵ��   : " << g_Bssrdf.scatter.r << ", " << 
            g_Bssrdf.scatter.g << ", " << 
            g_Bssrdf.scatter.b << endl;
        cout << "    ����ϵ��   : " << g_Bssrdf.absorb.r << ", " << 
            g_Bssrdf.absorb.g << ", " << 
            g_Bssrdf.absorb.b << endl;
        cout << "    ������     : " << g_Bssrdf.eta << endl;
        cout << "    ɢ���ϵ�� : " << g_Bssrdf.g << endl;
    }
}

void CCommandThread::cmdSet( const char* params )
{
    vector<string> listCmd = ParseCmdMultiParam(params, (int)strlen(params));
    if (listCmd.size() < 1)
    {
        cout << "δ���������\n";
    }
    else if (listCmd.size() < 2)
    {
        cout << "δ����ֵ\n";
    }
    else
    {
        if (listCmd[0] == "err")
        {
            g_RenderOption.maxErr = (float) atof(listCmd[1].c_str());
        }
        else if (listCmd[0] == "factor")
        {
            g_RenderOption.transFactor = (float) atof(listCmd[1].c_str());
        }
        else if (listCmd[0] == "s")
        {
            if (listCmd.size() != 4)
            {
                printf("ɢ��ϵ����Ҫ3��ֵ\n");
            }
            else
            {
                g_Bssrdf.scatter.r = (float) atof(listCmd[1].c_str());
                g_Bssrdf.scatter.g = (float) atof(listCmd[2].c_str());
                g_Bssrdf.scatter.b = (float) atof(listCmd[3].c_str());
            }
        }
        else if (listCmd[0] == "a")
        {
            if (listCmd.size() != 4)
            {
                printf("����ϵ����Ҫ3��ֵ\n");
            }
            else
            {
                g_Bssrdf.absorb.r = (float) atof(listCmd[1].c_str());
                g_Bssrdf.absorb.g = (float) atof(listCmd[2].c_str());
                g_Bssrdf.absorb.b = (float) atof(listCmd[3].c_str());
            }
        }
        else if (listCmd[0] == "g")
        {
            g_Bssrdf.g = (float) atof(listCmd[1].c_str());
        }
        else if (listCmd[0] == "eta")
        {
            g_Bssrdf.eta = (float) atof(listCmd[1].c_str());
        }
        else if (listCmd[0] == "scale")
        {
            g_RenderOption.scale = (float) atof(listCmd[1].c_str());
        }
        else
        {
            cout << "����ı�����\n";
        }
    }
}