#include "ServerSocket.h"
#include <process.h>
#include <assert.h>

static unsigned int __stdcall ThreadProc(void *lpParameter)
{
    ((CServerSocket *)lpParameter)->Run();

    return 0;
}

CServerSocket::CServerSocket()
{
    m_nPort = 0;
    m_bClosed = true;
    m_pListener = NULL;
    m_pSockets = NULL;
}

CServerSocket::~CServerSocket()
{
    Close();
}

void CServerSocket::SetListener(ISocketListener *pListener)
{
    m_pListener = pListener;
}

bool CServerSocket::Open(int nPort, int nMaxNum)
{
    struct sockaddr_in localAddr;
    int i;
    unsigned int id;

    Close();

    m_nPort = nPort;
    m_nMaxNum = nMaxNum;  //max number of concurrent clients
    m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (m_ServerSocket == INVALID_SOCKET) {
        return false;
    }

    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(m_nPort);
	localAddr.sin_addr.s_addr = inet_addr(g_sInAddr);
	if (localAddr.sin_addr.s_addr == INADDR_NONE) {
		g_sInAddr = "0.0.0.0";
		localAddr.sin_addr.s_addr = INADDR_ANY;
	}

    if (bind(m_ServerSocket, (struct sockaddr *)&localAddr, sizeof(localAddr)) == SOCKET_ERROR)	{
        closesocket(m_ServerSocket);

        return false;
    }

    if (listen(m_ServerSocket, m_nMaxNum) == SOCKET_ERROR) {
        closesocket(m_ServerSocket);

        return false;
    }

    m_pSockets = new CSocket *[m_nMaxNum];

    for (i = 0; i < m_nMaxNum; i++) {
        m_pSockets[i] = new CSocket(SOCK_STREAM);
    }

    m_bClosed = false;
    m_hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, this, 0, &id);  //begin thread

    return true;
}

void CServerSocket::Close(void)
{
    int i;

    if (m_bClosed) {
        return;
    }

    m_bClosed = true;

    closesocket(m_ServerSocket);

    if (m_hThread != NULL) {
        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
    }

    if (m_pSockets != NULL) {
        for (i = 0; i < m_nMaxNum; i++) {
            delete m_pSockets[i];
        }

        delete[] m_pSockets;
        m_pSockets = NULL;
    }
}

int CServerSocket::GetPort(void)
{
    return m_nPort;
}

void CServerSocket::Run(void)
{
    int i, nSize;
    struct sockaddr_in remoteAddr;
    SOCKET socket;

    nSize = sizeof(remoteAddr);

    while (!m_bClosed) {
        socket = accept(m_ServerSocket, (struct sockaddr *)&remoteAddr, &nSize);  //accept connection

        if (socket != INVALID_SOCKET) {
            for (i = 0; i < m_nMaxNum; i++) {
                if (!m_pSockets[i]->Active()) { //find an inactive CSocket
                    m_pSockets[i]->Open(socket, m_pListener, &remoteAddr);  //receive input data
                    break;
                }
            }
        }

    }
}
