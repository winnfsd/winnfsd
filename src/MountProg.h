#ifndef _MOUNTPROG_H_
#define _MOUNTPROG_H_

#include "RPCProg.h"

#define MOUNT_NUM_MAX 100
#define MOUNT_PATH_MAX 100

class CMountProg : public CRPCProg
{
    public:
    CMountProg();
    virtual ~CMountProg();
    void Export(char *path, char *pathAlias);
    char *GetClientAddr(int nIndex);
    int GetMountNumber(void);
    int Process(IInputStream *pInStream, IOutputStream *pOutStream, ProcessParam *pParam);

    protected:
    int m_nMountNum;
    int m_nPathNum;
    char m_pExportPaths[MOUNT_PATH_MAX][MAXPATHLEN];
    char m_pPathAliases[MOUNT_PATH_MAX][MAXPATHLEN];
    char *m_pClientAddr[MOUNT_NUM_MAX];
    IInputStream *m_pInStream;
    IOutputStream *m_pOutStream;

    void ProcedureNULL(void);
    void ProcedureMNT(void);
    void ProcedureUMNT(void);
    void ProcedureNOIMP(void);

    private:
    ProcessParam *m_pParam;
    int m_nResult;

    char *GetPath(int &pathNumber);
};

#endif
