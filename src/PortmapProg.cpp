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

    if (pParam->nProc == MAPPROC_GETPORT) {
        ProcedureGETPORT();
        PrintLog("\n");
        return PRC_OK;
    } else if  (pParam->nProc == MAPPROC_DUMP) {
        ProcedureDUMP();
        PrintLog("\n");
        return PRC_OK;
    } else if (pParam->nProc == MAPPROC_NULL) {
        PrintLog("MAPPROC_NULL\n");
        return PRC_OK;
    } else {
        PrintLog("NOTIMP\n");
        return PRC_NOTIMP;
    }

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

bool CPortmapProg::ProcedureDUMP(void)
{
    PORTMAP_HEADER header;

    PrintLog("DUMP");

    m_pOutStream->Write(1);

    header.prog = PROG_PORTMAP;
    header.vers = 2;
    header.proto = IPPROTO_TCP;
    header.port = PORTMAP_PORT;
    m_pOutStream->Write(header.prog);
    m_pOutStream->Write(header.vers);
    m_pOutStream->Write(header.proto);
    m_pOutStream->Write(header.port);

    m_pOutStream->Write(1);

    header.proto = IPPROTO_UDP;
    m_pOutStream->Write(header.prog);
    m_pOutStream->Write(header.vers);
    m_pOutStream->Write(header.proto);
    m_pOutStream->Write(header.port);

    m_pOutStream->Write(1);

    header.prog = PROG_NFS;
    header.vers = 3;
    header.proto = IPPROTO_TCP;
    header.port = NFS_PORT;
    m_pOutStream->Write(header.prog);
    m_pOutStream->Write(header.vers);
    m_pOutStream->Write(header.proto);
    m_pOutStream->Write(header.port);
    m_pOutStream->Write(1);

    header.proto = IPPROTO_UDP;
    m_pOutStream->Write(header.prog);
    m_pOutStream->Write(header.vers);
    m_pOutStream->Write(header.proto);
    m_pOutStream->Write(header.port);

    m_pOutStream->Write(1);

    header.prog = PROG_MOUNT;
    header.vers = 3;
    header.proto = IPPROTO_TCP;
    header.port = MOUNT_PORT;
    m_pOutStream->Write(header.prog);
    m_pOutStream->Write(header.vers);
    m_pOutStream->Write(header.proto);
    m_pOutStream->Write(header.port);

    m_pOutStream->Write(1);

    header.proto = IPPROTO_UDP;
    m_pOutStream->Write(header.prog);
    m_pOutStream->Write(header.vers);
    m_pOutStream->Write(header.proto);
    m_pOutStream->Write(header.port);

    m_pOutStream->Write(0);

    return true;
}