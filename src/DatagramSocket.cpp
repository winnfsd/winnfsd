#include "DatagramSocket.h"

CDatagramSocket::CDatagramSocket()
{
    m_nPort = 0;
    m_bClosed = true;
    m_pSocket = NULL;
}

CDatagramSocket::~CDatagramSocket()
{
    Close();
}

void CDatagramSocket::SetListener(ISocketListener *pListener)
{
    m_pListener = pListener;
}

bool CDatagramSocket::Open(int nPort)
{
    struct sockaddr_in localAddr;

    Close();

    m_nPort = nPort;
    m_Socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (m_Socket == INVALID_SOCKET) {
        return false;
    }

    int snd_buffer = 1024 * 1024 * 1;
    int snd_buffer_sizeof = (int)sizeof(snd_buffer);
    setsockopt(m_Socket, SOL_SOCKET, SO_SNDBUF, (char*)&snd_buffer, snd_buffer_sizeof);
    int rcv_buffer = 1024 * 1024 * 8;
    int rcv_buffer_sizeof = (int)sizeof(rcv_buffer);
    setsockopt(m_Socket, SOL_SOCKET, SO_RCVBUF, (char*)&rcv_buffer, rcv_buffer_sizeof);

    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(m_nPort);
	localAddr.sin_addr.s_addr = inet_addr(g_sInAddr);
	if (localAddr.sin_addr.s_addr == INADDR_NONE) {
		g_sInAddr = "0.0.0.0";
		localAddr.sin_addr.s_addr = INADDR_ANY;
	}

    if (bind(m_Socket, (struct sockaddr *)&localAddr, sizeof(struct sockaddr)) == SOCKET_ERROR) {
        closesocket(m_Socket);
        return false;
    }

    m_bClosed = false;
    m_pSocket = new CSocket(SOCK_DGRAM);
    m_pSocket->Open(m_Socket, m_pListener);  //wait for receiving data
    return true;
}

void CDatagramSocket::Close(void)
{
    if (m_bClosed) {
        return;
    }

    m_bClosed = true;
    delete m_pSocket;
}

int CDatagramSocket::GetPort(void)
{
    return m_nPort;
}
