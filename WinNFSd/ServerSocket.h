#ifndef _SERVERSOCKET_H_
#define _SERVERSOCKET_H_

#include "SocketListener.h"
#include "Socket.h"
#include <winsock.h>

class CServerSocket
{
    public:
    CServerSocket();
    ~CServerSocket();
    void SetListener(ISocketListener *pListener);
    bool Open(int nPort, int nMaxNum);
    void Close(void);
    int GetPort(void);
    void Run(void);

    private:
    int m_nPort, m_nMaxNum;
    SOCKET m_ServerSocket;
    bool m_bClosed;
    ISocketListener *m_pListener;
    HANDLE m_hThread;
    CSocket **m_pSockets;
};

#endif
