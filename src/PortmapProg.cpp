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
    PrintLog("PORTMAP ");
    m_pInStream = pInStream;
    m_pOutStream = pOutStream;

    if (pParam->nProc != MAPPROC_GETPORT) {
        PrintLog("NOTIMP\n");
        return PRC_NOTIMP;
    }

    ProcedureGETPORT();
    PrintLog("\n");

    return PRC_OK;
}

bool CPortmapProg::ProcedureGETPORT(void)
{
    PORTMAP_HEADER header;
    unsigned long nPort;

    PrintLog("GETPORT");
    m_pInStream->Read(&header.prog);  //program
    m_pInStream->Skip(12);
    nPort = header.prog >= MIN_PROG_NUM && header.prog < MIN_PROG_NUM + PORT_NUM ? m_nPortTable[header.prog - MIN_PROG_NUM] : 0;
    PrintLog(" %d %d", header.prog, nPort);
    m_pOutStream->Write(nPort);  //port

    return true;
}
