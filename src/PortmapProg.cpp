#include "PortmapProg.h"
#include <string.h>

#define MIN_PROG_NUM 100000
enum
{
    MAPPROC_NULL = 0,
    MAPPROC_SET = 1,
    MAPPROC_UNSET = 2,
    MAPPROC_GETPORT = 3,
    MAPPROC_DUMP = 4,
    MAPPROC_CALLIT = 5
};

enum
{
    IPPROTO_TCP = 6,
    IPPROTO_UDP = 17
};

enum
{
    PORTMAP_PORT = 111,
    MOUNT_PORT = 1058,
    NFS_PORT = 2049
};

enum
{
    PROG_PORTMAP = 100000,
    PROG_NFS = 100003,
    PROG_MOUNT = 100005
};

typedef void (CPortmapProg::*PPROC)(void);

CPortmapProg::CPortmapProg() : CRPCProg()
{
    memset(m_nPortTable, 0, PORT_NUM * sizeof(int));
}

CPortmapProg::~CPortmapProg()
{
}

void CPortmapProg::Set(unsigned long nProg, unsigned long nPort)
{
    m_nPortTable[nProg - MIN_PROG_NUM] = nPort;  //set port for program
}

int CPortmapProg::Process(IInputStream *pInStream, IOutputStream *pOutStream, ProcessParam *pParam)
{
    static PPROC pf[] = {
        &CPortmapProg::ProcedureNULL, &CPortmapProg::ProcedureSET, &CPortmapProg::ProcedureUNSET,
        &CPortmapProg::ProcedureGETPORT, &CPortmapProg::ProcedureDUMP, &CPortmapProg::ProcedureCALLIT
    };

    PrintLog("PORTMAP ");

    if (pParam->nProc >= sizeof(pf) / sizeof(PPROC)) {
        ProcedureNOIMP();
        PrintLog("\n");
        return PRC_NOTIMP;
    }

    m_pInStream = pInStream;
    m_pOutStream = pOutStream;
    m_pParam = pParam;
    m_nResult = PRC_OK;
    (this->*pf[pParam->nProc])();
    PrintLog("\n");

    return m_nResult;
}

void CPortmapProg::ProcedureNOIMP(void)
{
    PrintLog("NOIMP");
    m_nResult = PRC_NOTIMP;
}

void CPortmapProg::ProcedureNULL(void)
{
    PrintLog("NULL");
}

void CPortmapProg::ProcedureSET(void)
{
    PrintLog("SET - NOIMP");
    m_nResult = PRC_NOTIMP;
}

void CPortmapProg::ProcedureUNSET(void)
{
    PrintLog("UNSET - NOIMP");
    m_nResult = PRC_NOTIMP;
}

void CPortmapProg::ProcedureGETPORT(void)
{
    PORTMAP_HEADER header;
    unsigned long nPort;

    PrintLog("GETPORT");
    m_pInStream->Read(&header.prog);  //program
    m_pInStream->Skip(12);
    nPort = header.prog >= MIN_PROG_NUM && header.prog < MIN_PROG_NUM + PORT_NUM ? m_nPortTable[header.prog - MIN_PROG_NUM] : 0;
    PrintLog(" %d %d", header.prog, nPort);
    m_pOutStream->Write(nPort);  //port
}

void CPortmapProg::ProcedureDUMP(void)
{
    PrintLog("DUMP");

    Write(PROG_PORTMAP, 2, IPPROTO_TCP, PORTMAP_PORT);
    Write(PROG_PORTMAP, 2, IPPROTO_UDP, PORTMAP_PORT);
    Write(PROG_NFS, 3, IPPROTO_TCP, NFS_PORT);
    Write(PROG_NFS, 3, IPPROTO_UDP, NFS_PORT);
    Write(PROG_MOUNT, 3, IPPROTO_TCP, MOUNT_PORT);
    Write(PROG_MOUNT, 3, IPPROTO_UDP, MOUNT_PORT);

    m_pOutStream->Write(0);
}

void CPortmapProg::ProcedureCALLIT(void)
{
    PrintLog("CALLIT - NOIMP");
    m_nResult = PRC_NOTIMP;
}

void CPortmapProg::Write(unsigned long prog, unsigned long vers, unsigned long proto, unsigned long port)
{
    m_pOutStream->Write(1);
    m_pOutStream->Write(prog);
    m_pOutStream->Write(vers);
    m_pOutStream->Write(proto);
    m_pOutStream->Write(port);
}