#include "Socket.h"
#include <process.h>

char *g_sInAddr;

static unsigned int __stdcall ThreadProc(void *lpParameter)
{
    CSocket *pSocket;

    pSocket = (CSocket *)lpParameter;
    pSocket->Run();

    return 0;
}

CSocket::CSocket(int nType)
{
    m_nType = nType;  //socket type
    m_Socket = INVALID_SOCKET;
    memset(&m_RemoteAddr, 0, sizeof(m_RemoteAddr));
    m_bActive = false;
    m_hThread = NULL;
}

CSocket::~CSocket()
{
    Close();
}

int CSocket::GetType(void)
{
    return m_nType;  //socket type
}

void CSocket::Open(SOCKET socket, ISocketListener *pListener, struct sockaddr_in *pRemoteAddr)
{
    unsigned int id;

    Close();

    m_Socket = socket;  //socket
    m_pListener = pListener;  //listener

    if (pRemoteAddr != NULL) {
        m_RemoteAddr = *pRemoteAddr;  //remote address
    }

    if (m_Socket != INVALID_SOCKET) {
        m_bActive = true;
        m_hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, this, 0, &id);  //begin thread
    }
}

void CSocket::Close(void)
{
    if (m_Socket != INVALID_SOCKET) {
        closesocket(m_Socket);
        m_Socket = INVALID_SOCKET;
    }

    if (m_hThread != NULL) {
        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }
}

void CSocket::Send(void)
{
    if (m_Socket == INVALID_SOCKET)
        return;

    if (m_nType == SOCK_STREAM) {
        send(m_Socket, (const char *)m_SocketStream.GetOutput(), m_SocketStream.GetOutputSize(), 0);
    } else if (m_nType == SOCK_DGRAM) {
        sendto(m_Socket, (const char *)m_SocketStream.GetOutput(), m_SocketStream.GetOutputSize(), 0, (struct sockaddr *)&m_RemoteAddr, sizeof(struct sockaddr));
    }

    m_SocketStream.Reset();  //clear output buffer
}

bool CSocket::Active(void)
{
    return m_bActive;  //thread is active or not
}

char *CSocket::GetRemoteAddress(void)
{
    return inet_ntoa(m_RemoteAddr.sin_addr);
}

int CSocket::GetRemotePort(void)
{
    return htons(m_RemoteAddr.sin_port);
}

IInputStream *CSocket::GetInputStream(void)
{
    return &m_SocketStream;
}

IOutputStream *CSocket::GetOutputStream(void)
{
    return &m_SocketStream;
}

void CSocket::Run(void)
{
    int nSize, nBytes, fragmentHeaderMsb, fragmentHeaderLengthBytes;
    unsigned long fragmentHeader;

    nSize = sizeof(m_RemoteAddr);

    for (;;) {
        if (m_nType == SOCK_STREAM) {
            // When using tcp we cannot ensure that everything we need is already
            // received. When using RCP over TCP a fragment header is added to
            // work around this. The MSB of the fragment header determines if the
            // fragment is complete (not used here) and the remaining bits define the
            // length of the rpc call (this is what we want)
            nBytes = recv(m_Socket, (char *)m_SocketStream.GetInput(), 4, MSG_PEEK);

            // only if at least 4 bytes are availabe (the fragment header) we can continue
            if (nBytes == 4){
                m_SocketStream.SetInputSize(4);
                m_SocketStream.Read(&fragmentHeader);
                fragmentHeaderMsb = (int)(fragmentHeader & 0x80000000);
                fragmentHeaderLengthBytes = (int)(fragmentHeader ^ 0x80000000) + 4;
                while (nBytes != fragmentHeaderLengthBytes) {
                    nBytes = recv(m_Socket, (char *)m_SocketStream.GetInput(), fragmentHeaderLengthBytes, MSG_PEEK);
                }
                nBytes = recv(m_Socket, (char *)m_SocketStream.GetInput(), fragmentHeaderLengthBytes, 0);
            } else {
                nBytes = 0;
            }
        } else if (m_nType == SOCK_DGRAM) {
            nBytes = recvfrom(m_Socket, (char *)m_SocketStream.GetInput(), m_SocketStream.GetBufferSize(), 0, (struct sockaddr *)&m_RemoteAddr, &nSize);
        }


        if (nBytes > 0) {
            m_SocketStream.SetInputSize(nBytes);  //bytes received

            if (m_pListener != NULL) {
                m_pListener->SocketReceived(this);  //notify listener
            }
        } else {
            break;
        }
    }

    m_bActive = false;
}
