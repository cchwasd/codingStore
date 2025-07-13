#include "mulThreadServer.h"
#include <iostream>

SocketServerTest::SocketServerTest():m_nServerSocket(-1)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		OutputMessage("Socket�汾����");
}

SocketServerTest::~SocketServerTest()
{
	CloseMySocket();
}

void SocketServerTest::CloseMySocket()
{
	//�˳������߳�
	for (auto it : m_Vecthread)
	{
		it->isRuning = false;
	}

	if (m_nServerSocket != -1)
		closesocket(m_nServerSocket);	//�ر�socket����

	m_nServerSocket = -1;
	WSACleanup();	//��ֹws2_32.lib��ʹ��
}

bool SocketServerTest::CreateSocket()
{
	if (m_nServerSocket == -1)
	{
		m_nServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	//�趨TCPЭ��ӿ�;ʧ�ܷ���INVALID_SOCKET
		if (m_nServerSocket != INVALID_SOCKET) 
		{
			OutputMessage("�����������ɹ�");
			return true;
		}
	}
	return false;
}

bool SocketServerTest::BandSocket(const char* ip, const unsigned short prot)
{
	int nRet = -1;
	if (m_nServerSocket != -1)
	{
		sockaddr_in Serveraddr;
		memset(&Serveraddr, 0, sizeof(sockaddr_in*));
		Serveraddr.sin_family = AF_INET;
		Serveraddr.sin_addr.s_addr = inet_addr(ip);
		Serveraddr.sin_port = htons(prot);
		nRet = bind(m_nServerSocket, (sockaddr *)&Serveraddr, sizeof(Serveraddr));	//�󶨷�������ַ�Ͷ˿ں�;�ɹ�������0������ΪSOCKET_ERROR
	}

	if (nRet == 0)
	{
		OutputMessage("��IP�Ͷ˿ڳɹ�");
		return true;
	}

	OutputMessage("��IP�Ͷ˿�ʧ��");
	return false;
}

bool SocketServerTest::ListenSocket()
{
	int nRet = -1;
	if (m_nServerSocket != -1)
	{
		nRet = listen(m_nServerSocket, 5);//�趨�������ӵ��׽��֣��Լ��趨���Ӷ��г���;�ɹ�����0��ʧ�ܷ���-1
	}
	if (nRet == SOCKET_ERROR)
	{
		OutputMessage("������ʧ��");
		return false;
	}
		
	OutputMessage("�����󶨳ɹ�");
	return true;
}

void SocketServerTest::AcceptSocketManager()
{
	while (m_nServerSocket != -1)
	{
		sockaddr_in nClientSocket;//���Ҫ����ͻ��˵�IP�Ͷ˿ںţ��ʹ��ڱ���
		int nSizeClient = sizeof(nClientSocket);
		SOCKET sClient = accept(m_nServerSocket, (sockaddr*)&nClientSocket, &nSizeClient);//���ܿͻ������ӣ�����״̬;ʧ�ܷ���-1
		if (sClient == SOCKET_ERROR)
		{
			OutputMessage("��ǰ��ͻ�������ʧ��");
			return;
		}
		else
		{
			AddClientSocket(sClient);
		}
		Sleep(25);
	}
}

void SocketServerTest::AddClientSocket(SOCKET& sClient)
{
	Sthread *it = new Sthread();
	it->threadID = ++m_CsocketCount;
	it->isRuning = true;
	it->csocket = sClient;
	std::thread t(&SocketServerTest::ThreadClientRecv, this, it);
	t.detach();
	it->t1 = &t;
	m_Vecthread.push_back(it);
	char str[50];
	sprintf_s(str, "%dthread connect is success", it->threadID);
	OutputMessage(str);

	char mess[] = "sercer:����������ӳɹ���";
	send(sClient, mess, sizeof(mess), 0);//������Ϣ���ͻ���
}

void SocketServerTest::ThreadClientRecv(Sthread *sthread)
{
	while (sthread->isRuning == true)
	{
		// �ӿͻ��˽�������
		char buff[65535];
		int nRecv = recv(sthread->csocket, buff, 65535, 0);//�ӿͻ��˽�����Ϣ
		if (nRecv > 0)
		{
			char str[50];
			sprintf_s(str, "%dthread send message", sthread->threadID);
			OutputMessage(str);
			OutputMessage(buff);
			char mess[] = "server:�յ��������Ϣ��";
			send(sthread->csocket, mess, sizeof(mess), 0);
		}
		else
		{
			char str[50];
			sprintf_s(str, "ID%d is exit", sthread->threadID);
			OutputMessage(str);
			sthread->isRuning = false;
		}
	}
	return;
}

void SocketServerTest::OutputMessage(const char *outstr)
{
	std::cout << outstr << std::endl;
}
