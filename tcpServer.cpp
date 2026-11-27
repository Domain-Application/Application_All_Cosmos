//Notes: docs/CPP_PROGRAMMING_NOTES.md.
#include "stdafx.h"
#include "TcpServer.h"

static int WaitReadable(SOCKET s, int ms)
{
    fd_set set;
    FD_ZERO(&set);
    FD_SET(s, &set);
    timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    return select(0, &set, NULL, NULL, &tv);   // 1 ready, 0 timeout, SOCKET_ERROR
}

CTcpServer::CTcpServer()
{
    m_listen = INVALID_SOCKET;
    m_client = INVALID_SOCKET;
}

CTcpServer::~CTcpServer()
{
    Close();
}

BOOL CTcpServer::StartServer(int port)
{
    m_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listen == INVALID_SOCKET) return FALSE;

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((u_short)port);

    if (bind(m_listen, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR ||
        listen(m_listen, SOMAXCONN) == SOCKET_ERROR)
    {
        Close();
        return FALSE;
    }
    return TRUE;
}

int CTcpServer::WaitClient(int ms)
{
    int ready = WaitReadable(m_listen, ms);
    if (ready <= 0) return ready == 0 ? 0 : -1;

    m_client = accept(m_listen, NULL, NULL);
    return (m_client == INVALID_SOCKET) ? -1 : 1;
}

int CTcpServer::ReceiveData(CString &data, int ms)
{
    int ready = WaitReadable(m_client, ms);
    if (ready == 0) return 0;
    if (ready < 0) return -1;

    char buffer[2048];
    int len = recv(m_client, buffer, sizeof(buffer), 0);
    if (len <= 0) return -1;

    data = CString(buffer, len);
    return len;
}

void CTcpServer::CloseClient()
{
    if (m_client != INVALID_SOCKET)
    {
        closesocket(m_client);
        m_client = INVALID_SOCKET;
    }
}

void CTcpServer::Close()
{
    CloseClient();
    if (m_listen != INVALID_SOCKET)
    {
        closesocket(m_listen);
        m_listen = INVALID_SOCKET;
    }
}
