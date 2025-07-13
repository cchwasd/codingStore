//multipartiteClientSocket.h
// #pragma once
#ifndef _MULTHREADCLIENT_H
#define _MULTHREADCLIENT_H
#include <WinSock2.h>
#include <iostream>
#include <thread>

// #pragma comment(lib, "ws2_32.lib")  //���� ws2_32.dll

typedef struct thread1
{
	std::thread *t1 = nullptr;
	bool isRuning = false;
} Mythread;

class MultipartiteClientSocketTest
{
public:
	MultipartiteClientSocketTest();
	~MultipartiteClientSocketTest();

	bool CreateSocket();
	void CloseSocket();

	bool Myconnect(const char* ip, const unsigned short prot);
	void Mysend();
	void Myrecv();
	bool InitMyrecv();

	void OutputMessage(const char* outstr);

private:
	char m_message[256];

public:
	SOCKET m_nLocalSocket;
private:
	Mythread recvThread;
};
#endif //_MULTHREADCLIENT_H