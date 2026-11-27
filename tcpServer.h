#pragma once
#include <afxsock.h>

//ANPR listener socket: every wait has a timeout, so the thread can check its stop flag.
class CTcpServer
{
public:
    CTcpServer();
    ~CTcpServer();

    BOOL StartServer(int port);
    int  WaitClient(int ms);                    // 1 client accepted, 0 timeout, -1 error
    int  ReceiveData(CString &data, int ms);    // >0 bytes, 0 timeout, -1 client gone
    void CloseClient();
    void Close();

private:
    SOCKET m_listen;
    SOCKET m_client;
};
