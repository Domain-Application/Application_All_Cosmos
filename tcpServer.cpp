#include "stdafx.h"
#include "TcpServer.h"

BOOL CTcpServer::StartServer(int port)
{
    if (!serverSocket.Create(port, SOCK_STREAM))
    {
        AfxMessageBox(_T("Failed to create TCP server socket"));
        return FALSE;
    }

    if (!serverSocket.Listen())
    {
        AfxMessageBox(_T("Failed to listen on port"));
        return FALSE;
    }

    return TRUE;
}

BOOL CTcpServer::AcceptClient()
{
    if (!serverSocket.Accept(clientSocket))
    {
        AfxMessageBox(_T("Failed to accept incoming client"));
        return FALSE;
    }

    return TRUE;
}

int CTcpServer::ReceiveData(CString &data)
{
    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));

    int len = clientSocket.Receive(buffer, sizeof(buffer)-1);

    if (len > 0)
    {
        data = buffer;
    }

    return len;
}

void CTcpServer::SendData(const CString &data)
{
    // CString to ANSI char buffer
    int len = data.GetLength();
    char* buffer = new char[len + 1];

    for (int i = 0; i < len; i++)
        buffer[i] = (char)data[i];

    buffer[len] = '\0';

    clientSocket.Send(buffer, len);

    delete[] buffer;
}

void CTcpServer::CloseClient()
{
    try { clientSocket.Close(); } catch(...) {}
}

void CTcpServer::Close()
{
    try { clientSocket.Close(); } catch(...) {}
    try { serverSocket.Close(); } catch(...) {}
}
