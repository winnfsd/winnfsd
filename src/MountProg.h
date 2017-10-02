#ifndef _MOUNTPROG_H_
#define _MOUNTPROG_H_

#include "RPCProg.h"
#include <map>
#include <string>

#define MOUNT_NUM_MAX 100
#define MOUNT_PATH_MAX 100

enum pathFormats
{
	FORMAT_PATH = 1,
	FORMAT_PATHALIAS = 2
};

class CMountProg : public CRPCProg
{
    public:
    CMountProg();
    virtual ~CMountProg();
	bool SetPathFile(const char *file);
    void Export(const char *path, const char *pathAlias);
	bool Refresh();
    char *GetClientAddr(int nIndex);
    int GetMountNumber(void);
    int Process(IInputStream *pInStream, IOutputStream *pOutStream, ProcessParam *pParam);
	char *FormatPath(const char *pPath, pathFormats format);

    protected:
    int m_nMountNum;
	char *m_pPathFile;
	std::map<std::string, std::string> m_PathMap;
    char *m_pClientAddr[MOUNT_NUM_MAX];
    IInputStream *m_pInStream;
    IOutputStream *m_pOutStream;

    void ProcedureNULL(void);
    void ProcedureMNT(void);
    void ProcedureUMNT(void);
    void ProcedureUMNTALL(void);
    void ProcedureEXPORT(void);
    void ProcedureNOIMP(void);

    private:
    ProcessParam *m_pParam;
    int m_nResult;

	bool GetPath(char **returnPath);
    char *GetPath(int &pathNumber);
	bool ReadPathsFromFile(const char* sFileName);
};

#endif
