#ifndef _RPCSERVER_H_
#define _RPCSERVER_H_

#include "SocketListener.h"
#include "Socket.h"
#include "RPCProg.h"
#include <windows.h>
#include <deque>
#include <string>
#include <vector>

#define PROG_NUM 10

class CRPCServer : public ISocketListener
{
    public:
    CRPCServer();
    virtual ~CRPCServer();
    void Set(int nProg, CRPCProg *pRPCProg);
    void SetLogOn(bool bLogOn);
    void SocketReceived(CSocket *pSocket);

    protected:
    CRPCProg *m_pProgTable[PROG_NUM];
    HANDLE m_hMutex;
    bool m_bLogOn;

    typedef struct
    {
        std::string addr;
        unsigned long xid, prog, vers, proc, argsSize;
        ULONGLONG time;
        std::vector<unsigned char> reply;
    } REPLY_CACHE_ENTRY;

    std::deque<REPLY_CACHE_ENTRY> m_ReplyCache;

    int Process(int nType, IInputStream *pInStream, IOutputStream *pOutStream, char *pRemoteAddr);
};

#endif
