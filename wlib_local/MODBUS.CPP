// modbus.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "modbus.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int m_tid =0, _modbus_busy_now=0,_modbus_error_code=0,_modbus_fine_counter=0,_modbus_fail_counter=0;
int  __iOnModbusTCP = 0;
/********************************************************************************************

  MBAP Header
  
A new 7-byte header called the MBAP header (Modbus Application Header) is added to the start of the message.  
This header has the following data:
Transaction Identifier: 2 bytes set by the Client to uniquely identify each request. 
These bytes are echoed by the Server since its responses may not be received in the same order as the requests.
Protocol Identifier: 2 bytes set by the Client, always = 00 00
Length:  2 bytes identifying the number of bytes in the message to follow.
Unit Identifier: 1 byte set by the Client and echoed by the Server for identification of a remote slave connected on a serial line or on other buses.

0001 0000 0006 11 03 006B 0003

0001: Transaction Identifier
0000: Protocol Identifier
0006: Message Length (6 bytes to follow)
11: The Unit Identifier  (17 = 11 hex)
03: The Function Code (read Analog Output Holding Registers)
006B: The Data Address of the first register requested. (40108-40001 = 107 =6B hex)
0003: The total number of registers requested. (read 3 registers 40108 to 40110)


The different data types are directly related to the used function codes.
Modbus data type DATA_TYPE Function Length single_write Function code
Coils 1 reading any irrelevant 1
Coils 1 writing 1 TRUE 5
Coils 1 writing 1 FALSE 15
Coils 1 writing > 1 irrelevant 15
Inputs 2 reading any irrelevant 2
Holding Register 3 reading any irrelevant 3
Holding Register 3 writing 1 TRUE 6
Holding Register 3 writing 1 FALSE 16
Holding Register 3 writing > 1 irrelevant 16
Input Register 4 reading any irrelevant 4 
*******************************************************************************************/
///Function Code
#define READ_COILS 0x01
#define READ_INPUT_BITS 0x02
#define READ_REGS 0x03
#define READ_INPUT_REGS 0x04
#define WRITE_COIL 0x05
#define WRITE_REG 0x06
#define WRITE_COILS 0x0F
#define WRITE_REGS 0x10


static int _modbus_tcp_init_win32(void)
{
    /* Initialise Windows Socket API */
    WSADATA wsaData;
	
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        int error_io_no = GetLastError();
        fprintf(stderr, "WSAStartup() returned error code %d\n",
            (unsigned int)error_io_no);

        errno = error_io_no;
        return -1;
    }
    return 0;
}

/* Builds a TCP request header */
int _modbus_tcp_build_request_basis(int t_id, int slave, int function,
                                           int addr, int nb,
                                           BYTE *req)
{
    req[0] = t_id >> 8;
    req[1] = t_id & 0x00ff;

    /* Protocol Modbus */
    req[2] = 0;
    req[3] = 0;

    /* Length will be defined later by set_req_length_tcp at offsets 4
       and 5 */

	req[5] = 6;			//2022-07-07 connect with S7 1200 need assign length

	req[6] = slave;			 //0~247
    req[7] = function;       //01 read coil 02 read input status 05 write coil
    req[8] = addr >> 8;
    req[9] = addr & 0x00ff;
    req[10] = nb >> 8;
    req[11] = nb & 0x00ff;

    return 12;
}

int _modbus_tcp_prepare_response_tid(const int *req, int *req_length)
{
    return (req[0] << 8) + req[1];
}

/* Builds a TCP response header */
int _modbus_tcp_build_response_basis(int t_id,int slave, int function, BYTE *rsp)
{
    /* Extract from MODBUS Messaging on TCP/IP Implementation
       Guide V1.0b (page 23/46):
       The transaction identifier is used to associate the future
       response with the request. */
    rsp[0] = t_id >> 8;
    rsp[1] = t_id & 0x00ff;

    /* Protocol Modbus */
    rsp[2] = 0;
    rsp[3] = 0;

    /* Length will be set later by send_msg (4 and 5) */

    /* The slave ID is copied from the indication */
    rsp[6] = slave;
    rsp[7] = function;

    return TRUE;
}

int _modbus_tcp_send_msg_pre(BYTE *req, int req_length)
{
    /* Substract the header length to the message length */
    int mbap_length = req_length - 6;
	
    req[4] = mbap_length >> 8;
    req[5] = mbap_length & 0x00FF;
	
    return req_length;
}



int _modbus_tcp_pre_check_confirmation(const BYTE *req,
                                              const BYTE *rsp, int rsp_length,int debug)
{
    /* Check transaction ID */
    if (req[0] != rsp[0] || req[1] != rsp[1]) {
        if (debug) {
            fprintf(stderr, "Invalid transaction ID received 0x%X (not 0x%X)\n",
				(rsp[0] << 8) + rsp[1], (req[0] << 8) + req[1]);
        }
        errno = 0;
        return -1;
    }
	
    /* Check protocol ID */
    if (rsp[2] != 0x0 && rsp[3] != 0x0) {
        if (debug) {
            fprintf(stderr, "Invalid protocol ID received 0x%X (not 0x0)\n",
				(rsp[2] << 8) + rsp[3]);
        }
        errno = 1;
        return -1;
    }
	
    return 0;
}

#define MODBUS_TCP_MAX_ADU_LENGTH 1000

int _modbus_tcp_flush(int sock)
{
    int rc;
    int rc_sum = 0;
	
    do {
        /* Extract the garbage from the socket */
        char devnull[MODBUS_TCP_MAX_ADU_LENGTH];
#ifndef OS_WIN32
        rc = recv(sock, devnull, MODBUS_TCP_MAX_ADU_LENGTH, 0);
#else
        /* On Win32, it's a bit more complicated to not wait */
        fd_set rset;
        struct timeval tv;
		
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        FD_ZERO(&rset);
        FD_SET(ctx->s, &rset);
        rc = select(sock+1, &rset, NULL, NULL, &tv);
        if (rc == -1) {
            return -1;
        }
		
        if (rc == 1) {
            /* There is data to flush */
            rc = recv(sock, devnull, MODBUS_TCP_MAX_ADU_LENGTH, 0);
        }
#endif
        if (rc > 0) {
            rc_sum += rc;
        }
    } while (rc == MODBUS_TCP_MAX_ADU_LENGTH);
	
    return rc_sum;
}

void AddModbusLog(int sock,const char *msg)
{
    _modbus_busy_now=0;
    shutdown(sock,2);
    closesocket(sock);
    WSACleanup();
	_modbus_fail_counter++;

    FILE *ptr = log_file();
	if(!ptr) return;

	fprintf(ptr,"%s %s\n", nowMs(), msg);
	fclose(ptr);

    if(__iOnModbusTCP>=2) return;
    if(GetCurrentThreadId() != AfxGetApp()->m_nThreadID) return;  //no message box from a PLC polling thread

    MyMsgBox( msg, "Error", MB_ICONSTOP);
}

int __IsGoodModbusConnect()
{
    return (_modbus_fail_counter==0 && _modbus_fine_counter>100);
}

BOOL OnModbusTCP(const char *m_ip,UINT m_port,int &m_tid,int m_slave,int function, int m_add,int m_nb, int m_lb, BYTE *data,int buflen,int m_data)
{
    if(__iOnModbusTCP<2) __iOnModbusTCP++;    //only "first call" matters to AddModbusLog; stop before it wraps
    int error_connection_no = 0;
    _modbus_busy_now=1;

    CString m_result,m_respond;
	WSADATA ws;
	WSAStartup(MAKEWORD(2,2),&ws);
    
	struct sockaddr_in server;
	

    //Create socket
    int sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1) {
		_modbus_error_code = -1;
        error_connection_no = WSAGetLastError();
        m_result.Format("Could not create socket. IP %s err id:%d %i function:%i address:%i port:%i",
            m_ip,error_connection_no,_modbus_error_code, function, m_add, m_port);
        AddModbusLog(sock, m_result);
        return FALSE;
    }
	
    server.sin_addr.s_addr = inet_addr(m_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons( m_port );
	
    unsigned long flag=1;


    {   //always non-blocking: connect/send/recv waits are all bounded by select()
        if (ioctlsocket(sock,FIONBIO,&flag)!=0) {

            _modbus_error_code = -2;
            error_connection_no = WSAGetLastError();
            m_result.Format("Connection ioctlsocket failed. IP: %s err id:%d %i",
                m_ip,error_connection_no,_modbus_error_code);
             AddModbusLog(sock, m_result);

            return FALSE;
        }
    }

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {

        struct timeval timeout ;

        fd_set r;
        FD_ZERO(&r);
        FD_SET(sock,&r);
        timeout.tv_sec  = 2;
        timeout.tv_usec = 0;
        int ret = select(0,0,&r,0,&timeout);
        if ( ret <= 0 ) {

			_modbus_error_code = -3;
            error_connection_no = WSAGetLastError();
            m_result.Format("Connect fail. IP %s err id:%d %i function:%i address:%i port:%i",
                m_ip,error_connection_no,_modbus_error_code, function, m_add, m_port);
            AddModbusLog(sock, m_result);
            return FALSE;
        }
    }
	
	
	BYTE msg[1000] = {0};
    int nb = m_nb * 256 + m_lb;

	_modbus_tcp_build_request_basis(m_tid,m_slave, function, int(m_add), nb, msg);
	if(function == WRITE_COILS) {
		//Tx:000131-01 C7 00 00 00 09 01 0F 00 00 00 10 02 FF FF
		//Rx:000132-01 C7 00 00 00 06 01 0F 00 00 00 10

		int data_len = (nb + 7) / 8;
		msg[5] = data_len + 7;
		msg[12] = data_len;

		for(int i=0; i<data_len; i++) {
			msg[13+i] = m_data & 0xff;
			m_data = m_data >> 8;
		}
	}
	else if(function == WRITE_REGS) {
		//Tx:000145-02 5C 00 00 00 09 01 10 00 00 00 01 02 01 01
		//Rx:000146-02 5C 00 00 00 06 01 10 00 00 00 01
		int data_len = nb;
		msg[5] = data_len*2 + 7;
		msg[12] = data_len;

	}
	
	fd_set fds;
	FD_ZERO(&fds);
	timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	
    {
        // wait for permission to send
        FD_SET(sock, &fds);
        int i = select(32, NULL, &fds, NULL, &tv); // write
        if (i<=0) {
            _modbus_error_code = -4;
            error_connection_no = WSAGetLastError();
            m_result.Format("Select fail. IP %s err id:%d %i function:%i address:%i port:%i",
                m_ip,error_connection_no,_modbus_error_code, function, m_add, m_port);
            shutdown(sock,2);
            AddModbusLog(sock, m_result);
            return FALSE;
        }
    }

    int len = 12;				//modbus std length
	if(function==15) {
		len += msg[5] - 6;
	}
    int retval = send(sock, (char *) msg, len,0);
	if(retval != len) {
		_modbus_error_code = -5;
        error_connection_no = WSAGetLastError();
        m_result.Format("Send fail. IP %s err id:%d %i function:%i address:%i port:%i",
            m_ip,error_connection_no,_modbus_error_code, function, m_add, m_port);
        _modbus_busy_now=0;
        AddModbusLog(sock, m_result);
		return FALSE;
	}

    {
        // wait for response
        FD_SET(sock, &fds);
        int i = select(32, &fds, NULL, NULL, &tv); //read
        if (i<=0){
            _modbus_error_code = -6;
            error_connection_no = WSAGetLastError();
            m_result.Format("no TCP response received. IP %s err id:%d %i function:%i address:%i port:%i",
                m_ip,error_connection_no,_modbus_error_code, function, m_add, m_port);
            shutdown(sock,2);
            closesocket(sock);
            WSACleanup();
            _modbus_busy_now=0;
            return FALSE;
       }
    }

    nb = recv(sock, (char *) data,buflen,0 );
    if( nb < 0) {
        _modbus_error_code = -9;
        error_connection_no = WSAGetLastError();
        m_result.Format("Recv Error. IP %s err id:%d %i function:%i address:%i port:%i",
            m_ip,error_connection_no,_modbus_error_code, function, m_add, m_port);

        AddModbusLog(sock, m_result);
		return FALSE;
	}

	shutdown(sock,2);
    closesocket(sock);
    WSACleanup();
    m_tid++;
	_modbus_fine_counter++;
    _modbus_busy_now=0;
	_modbus_error_code = 0;
    return TRUE;
}

int Read_MODBUS_Analog_Data(const char *m_ip,int function,int nb, short int *value,UINT m_port)
{

    int m_slave = 1;

    int address = 0;

    BYTE data[1000] = {0};
    int flag2 = OnModbusTCP(m_ip,m_port,m_tid,m_slave, function,
                  address,0, nb, data, sizeof(data));

	for(int i=0; i<nb; i++) {
		int offset = i * 2;
        value[i] = data[9+offset] * 256 + data[10+offset];
	}

    return 1;
}
