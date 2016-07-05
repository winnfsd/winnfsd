#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "SocketListener.h"
#include "SocketStream.h"
#include <winsock.h>

extern char *g_sInAddr;

class CSocket
{
    public:
    CSocket(int nType);
    virtual ~CSocket();
    int GetType(void);
    void Open(SOCKET socket, ISocketListener *pListener, struct sockaddr_in *pRemoteAddr = NULL);
    void Close(void);
    void Send(void);
    bool Active(void);
    char *GetRemoteAddress(void);
    int GetRemotePort(void);
    IInputStream *GetInputStream(void);
    IOutputStream *GetOutputStream(void);
    void Run(void);

    private:
    int m_nType;
    SOCKET m_Socket;
    struct sockaddr_in m_RemoteAddr;
    ISocketListener *m_pListener;
    CSocketStream m_SocketStream;
    bool m_bActive;
    HANDLE m_hThread;
};

#endif
