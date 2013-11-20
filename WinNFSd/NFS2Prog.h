#ifndef _NFS2PROG_H_
#define _NFS2PROG_H_

#include "RPCProg.h"

class CNFS2Prog : public CRPCProg
{
    public:
    CNFS2Prog();
    ~CNFS2Prog();
    void SetUserID(unsigned int nUID, unsigned int nGID);
    int Process(IInputStream *pInStream, IOutputStream *pOutStream, ProcessParam *pParam);

    protected:
    unsigned int m_nUID, m_nGID;
    IInputStream *m_pInStream;
    IOutputStream *m_pOutStream;

    void ProcedureNULL(void);
    void ProcedureGETATTR(void);
    void ProcedureSETATTR(void);
    void ProcedureLOOKUP(void);
    void ProcedureREAD(void);
    void ProcedureWRITE(void);
    void ProcedureCREATE(void);
    void ProcedureREMOVE(void);
    void ProcedureRENAME(void);
    void ProcedureMKDIR(void);
    void ProcedureRMDIR(void);
    void ProcedureREADDIR(void);
    void ProcedureSTATFS(void);
    void ProcedureNOTIMP(void);

    private:
    int m_nResult;

    char *GetPath(void);
    char *GetFullPath(void);
    bool CheckFile(char *path);
    bool WriteFileAttributes(char *path);
};

#endif
