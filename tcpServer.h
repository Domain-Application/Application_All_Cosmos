#pragma once
#include <afxsock.h>

class CTcpServer
{
public:
    CSocket serverSocket;
    CSocket clientSocket;

public:
    BOOL StartServer(int port);
    BOOL AcceptClient();
    int  ReceiveData(CString &data);
    void SendData(const CString &data);
    void CloseClient();
    void Close();
};
