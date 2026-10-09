#include "RPCServer.h"
#include "ServerSocket.h"
#include <stdio.h>

#define MIN_PROG_NUM 100000
#define PROG_NFS 100003
#define REPLY_CACHE_SIZE 256
#define REPLY_CACHE_EXPIRE_MS (120 * 1000)  //same lifetime as Linux nfsd's RC_EXPIRE
enum
{
    CALL = 0,
    REPLY = 1
};

enum
{
    MSG_ACCEPTED = 0,
    MSG_DENIED = 1
};

enum
{
    SUCCESS = 0,
    PROG_UNAVAIL = 1,
    PROG_MISMATCH = 2,
    PROC_UNAVAIL = 3,
    GARBAGE_ARGS = 4
};

typedef struct
{
    unsigned long flavor;
    unsigned long length;
} OPAQUE_AUTH;

typedef struct
{
    unsigned long header;
    unsigned long XID;
    unsigned long msg;
    unsigned long rpcvers;
    unsigned long prog;
    unsigned long vers;
    unsigned long proc;
    OPAQUE_AUTH cred;
    OPAQUE_AUTH verf;
} RPC_HEADER;

CRPCServer::CRPCServer()
{
    int i;

    for (i = 0; i < PROG_NUM; i++) {
        m_pProgTable[i] = NULL;
    }
        
    m_hMutex = CreateMutex(NULL, false, NULL);
    m_bLogOn = true;
}

CRPCServer::~CRPCServer()
{
    CloseHandle(m_hMutex);
}

void CRPCServer::Set(int nProg, CRPCProg *pRPCProg)
{
    m_pProgTable[nProg - MIN_PROG_NUM] = pRPCProg;  //set program handler
}

void CRPCServer::SetLogOn(bool bLogOn)
{
    int i;

    m_bLogOn = bLogOn;

    for (i = 0; i < PROG_NUM; i++) {
        if (m_pProgTable[i] != NULL) {
            m_pProgTable[i]->SetLogOn(bLogOn);
        }
    }  
}

void CRPCServer::SocketReceived(CSocket *pSocket)
{
    IInputStream *pInStream;
    int nResult;

    WaitForSingleObject(m_hMutex, INFINITE);
    pInStream = pSocket->GetInputStream();

    while (pInStream->GetSize() > 0) {
        nResult = Process(pSocket->GetType(), pInStream, pSocket->GetOutputStream(), pSocket->GetRemoteAddress());  //process input data
        pSocket->Send();  //send response

        if (nResult != PRC_OK || pSocket->GetType() == SOCK_DGRAM) {
            break;
        }
    }

    ReleaseMutex(m_hMutex);
}

// NFSv3 procedures that must not run twice: SETATTR, CREATE, MKDIR, SYMLINK, MKNOD,
// REMOVE, RMDIR, RENAME, LINK. A replayed RENAME, for example, used to delete the
// file it had just renamed.
static bool IsNonIdempotent(unsigned long prog, unsigned long vers, unsigned long proc)
{
    return prog == PROG_NFS && vers == 3 && (proc == 2 || (proc >= 8 && proc <= 15));
}

int CRPCServer::Process(int nType, IInputStream *pInStream, IOutputStream *pOutStream, char *pRemoteAddr)
{
    RPC_HEADER header;
    int nPos, nSize, nStart;
    ProcessParam param;
    int nResult;
    bool bCache;
    unsigned long argsSize;
    ULONGLONG now;
    std::string remoteAddr(pRemoteAddr);

    nResult = PRC_OK;
    nStart = pOutStream->GetPosition();

    if (nType == SOCK_STREAM) {
        pInStream->Read(&header.header);
    }    

    pInStream->Read(&header.XID);
    pInStream->Read(&header.msg);
    pInStream->Read(&header.rpcvers);  //rpc version
    pInStream->Read(&header.prog);  //program
    pInStream->Read(&header.vers);  //program version
    pInStream->Read(&header.proc);  //procedure
    pInStream->Read(&header.cred.flavor);
    pInStream->Read(&header.cred.length);
    pInStream->Skip(header.cred.length);
    pInStream->Read(&header.verf.flavor);  //vefifier

    if (pInStream->Read(&header.verf.length) < sizeof(header.verf.length)) {
        nResult = PRC_FAIL;
    }
        
    if (pInStream->Skip(header.verf.length) < header.verf.length) {
        nResult = PRC_FAIL;
    }      

    // Duplicate request cache, like Linux nfsd's: a UDP client retransmits when a reply
    // is lost, so answer a repeated non-idempotent call with the reply it already got.
    // Entries expire after 120 s so a rebooted client that reuses an XID isn't answered
    // with an old reply.
    // ponytail: UDP only; the key compares the argument size where Linux checksums the
    // arguments, and has no client port. TCP clients retransmit only after a reconnect.
    bCache = nType == SOCK_DGRAM && nResult == PRC_OK && IsNonIdempotent(header.prog, header.vers, header.proc);
    argsSize = pInStream->GetSize();
    now = GetTickCount64();

    while (!m_ReplyCache.empty() && now - m_ReplyCache.front().time > REPLY_CACHE_EXPIRE_MS) {
        m_ReplyCache.pop_front();  //oldest first, so expired entries are always at the front
    }

    if (bCache) {
        for (size_t i = 0; i < m_ReplyCache.size(); i++) {
            REPLY_CACHE_ENTRY &entry = m_ReplyCache[i];

            if (entry.xid == header.XID && entry.proc == header.proc && entry.prog == header.prog &&
                entry.vers == header.vers && entry.argsSize == argsSize && entry.addr == remoteAddr) {
                pOutStream->Write(entry.reply.data(), entry.reply.size());
                pInStream->Skip(pInStream->GetSize());

                if (m_bLogOn) {
                    printf("Retransmitted request (XID %08lx, proc %lu) from %s answered from the reply cache\n", header.XID, header.proc, pRemoteAddr);
                }

                return PRC_OK;
            }
        }
    }

    if (nType == SOCK_STREAM) {
        nPos = pOutStream->GetPosition();  //remember current position
        pOutStream->Write(header.header);  //this value will be updated later
    }

    pOutStream->Write(header.XID);
    pOutStream->Write(REPLY);
    pOutStream->Write(MSG_ACCEPTED);
    pOutStream->Write(header.verf.flavor);
    pOutStream->Write(header.verf.length);

    if (nResult == PRC_FAIL) { //input data is truncated
        pOutStream->Write(GARBAGE_ARGS);
    } else if (header.prog < MIN_PROG_NUM || header.prog >= MIN_PROG_NUM + PROG_NUM) { //program is unavailable
        pOutStream->Write(PROG_UNAVAIL);
    } else if (m_pProgTable[header.prog - MIN_PROG_NUM] == NULL) { //program is unavailable
        pOutStream->Write(PROG_UNAVAIL);
    } else {
        pOutStream->Write(SUCCESS);  //this value may be modified later if process failed
        param.nVersion = header.vers;
        param.nProc = header.proc;
        param.pRemoteAddr = pRemoteAddr;
        nResult = m_pProgTable[header.prog - MIN_PROG_NUM]->Process(pInStream, pOutStream, &param);  //process rest input data by program

        if (nResult == PRC_NOTIMP) { //procedure is not implemented
            pOutStream->Seek(-4, SEEK_CUR);
            pOutStream->Write(PROC_UNAVAIL);
        } else if (nResult == PRC_FAIL) { //input data is truncated
            pOutStream->Seek(-4, SEEK_CUR);
            pOutStream->Write(GARBAGE_ARGS);
        }
    }

    if (nType == SOCK_STREAM) {
        nSize = pOutStream->GetPosition();  //remember current position
        pOutStream->Seek(nPos, SEEK_SET);  //seek to the position of head
        header.header = 0x80000000 + nSize - nPos - 4;  //size of output data
        pOutStream->Write(header.header);  //update header
    }

    if (bCache && nResult == PRC_OK) {
        REPLY_CACHE_ENTRY entry;
        entry.addr = remoteAddr;
        entry.xid = header.XID;
        entry.prog = header.prog;
        entry.vers = header.vers;
        entry.proc = header.proc;
        entry.argsSize = argsSize;
        entry.time = now;
        entry.reply.assign(pOutStream->GetOutput() + nStart, pOutStream->GetOutput() + pOutStream->GetPosition());
        m_ReplyCache.push_back(entry);

        if (m_ReplyCache.size() > REPLY_CACHE_SIZE) {
            m_ReplyCache.pop_front();
        }
    }

    return nResult;
}
