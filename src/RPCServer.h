#ifndef _RPCSERVER_H_
#define _RPCSERVER_H_

#include "SocketListener.h"
#include "Socket.h"
#include "RPCProg.h"
#include <windows.h>

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

    int Process(int nType, IInputStream *pInStream, IOutputStream *pOutStream, char *pRemoteAddr);
};

#endif
