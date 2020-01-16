#ifndef _PORTMAPPROG_H_
#define _PORTMAPPROG_H_

#include "RPCProg.h"

#define PORT_NUM 10

typedef struct
{
    unsigned long prog;
    unsigned long vers;
    unsigned long proto;
    unsigned long port;
} PORTMAP_HEADER;

class CPortmapProg : public CRPCProg
{
    public:
    CPortmapProg();
    virtual ~CPortmapProg();
    void Set(unsigned long nProg, unsigned long nPort);
    int Process(IInputStream *pInStream, IOutputStream *pOutStream, ProcessParam *pParam);

    protected:
    unsigned long m_nPortTable[PORT_NUM];
    IInputStream *m_pInStream;
    IOutputStream *m_pOutStream;

    void ProcedureNOIMP(void);
    void ProcedureNULL(void);
    void ProcedureSET(void);
    void ProcedureUNSET(void);
    void ProcedureGETPORT(void);
    void ProcedureDUMP(void);
    void ProcedureCALLIT(void);

    private:
    ProcessParam *m_pParam;
    int m_nResult;

    void Write(unsigned long prog, unsigned long vers, unsigned long proto, unsigned long port);
};

#endif
