#ifndef _SOCKETLISTENER_H_
#define _SOCKETLISTENER_H_

class CSocket;

class ISocketListener
{
    public:
    virtual void SocketReceived(CSocket *pSocket) = 0;
};

#endif
