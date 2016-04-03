#ifndef _NFSPROG_H_
#define _NFSPROG_H_

#include "RPCProg.h"
#include "NFS3Prog.h"

class CNFSProg : public CRPCProg
{
    public:
    CNFSProg();
    ~CNFSProg();
    void SetUserID(unsigned int nUID, unsigned int nGID);
    int Process(IInputStream *pInStream, IOutputStream *pOutStream, ProcessParam *pParam);
    void SetLogOn(bool bLogOn);

    private:
    unsigned int m_nUID, m_nGID;
    CNFS3Prog *m_pNFS3Prog;
};

#endif
