#include <iostream>
#include <thread>
#include <chrono>
#include "mulThreadServer.h"
#include "mulThreadClient.h"

// ���������Ժ���
void TestServer() {
    std::cout << "=== �������������� ===" << std::endl;
    
    SocketServerTest server;
    
    // ����Socket
    if (!server.CreateSocket()) {
        std::cout << "������Socket����ʧ�ܣ�" << std::endl;
        return;
    }
    
    // �󶨵�ַ�Ͷ˿�
    if (!server.BandSocket("127.0.0.1", 8888)) {
        std::cout << "��������ʧ�ܣ�" << std::endl;
        return;
    }
    
    // ��ʼ����
    if (!server.ListenSocket()) {
        std::cout << "����������ʧ�ܣ�" << std::endl;
        return;
    }
    
    std::cout << "�����������ɹ����ȴ��ͻ�������..." << std::endl;
    
    // �ں�̨�߳��д����ͻ�������
    std::thread acceptThread(&SocketServerTest::AcceptSocketManager, &server);
    acceptThread.detach();
    
    // ���̵߳ȴ�
    std::cout << "�������ֹͣ������..." << std::endl;
    std::cin.get();
}

// �ͻ��˲��Ժ���
void TestClient() {
    std::cout << "=== �����ͻ��˲��� ===" << std::endl;
    
    MultipartiteClientSocketTest client;
    
    // ����Socket
    if (!client.CreateSocket()) {
        std::cout << "�ͻ���Socket����ʧ�ܣ�" << std::endl;
        return;
    }
    
    // ���ӷ�����
    if (!client.Myconnect("127.0.0.1", 8888)) {
        std::cout << "���ӷ�����ʧ�ܣ�" << std::endl;
        return;
    }
    
    // �ͻ��˻��Զ���ʼ���ͺͽ�����Ϣ
    std::cout << "�ͻ������ӳɹ������Կ�ʼ������Ϣ..." << std::endl;
    std::cout << "������Ϣ���͸������������� 'quit' �˳���" << std::endl;
}

// ��ͻ��˲���
void TestMultipleClients() {
    std::cout << "=== ��ͻ��˲��� ===" << std::endl;
    
    // ����������
    std::thread serverThread([]() {
        SocketServerTest server;
        if (server.CreateSocket() && 
            server.BandSocket("127.0.0.1", 8888) && 
            server.ListenSocket()) {
            std::cout << "�����������ɹ�" << std::endl;
            server.AcceptSocketManager();
        }
    });
    serverThread.detach();
    
    // �ȴ�����������
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // ��������ͻ���
    for (int i = 1; i <= 3; i++) {
        std::thread clientThread([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(500 * i));
            
            MultipartiteClientSocketTest client;
            if (client.CreateSocket() && client.Myconnect("127.0.0.1", 8888)) {
                std::cout << "�ͻ��� " << i << " ���ӳɹ�" << std::endl;
                
                // ���ͼ���������Ϣ
                for (int j = 1; j <= 3; j++) {
                    std::string msg = "�ͻ���" + std::to_string(i) + "����Ϣ" + std::to_string(j);
                    send(client.m_nLocalSocket, msg.c_str(), msg.length(), 0);
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
        });
        clientThread.detach();
    }
    
    std::cout << "��ͻ��˲���������ɣ���������˳�..." << std::endl;
    std::cin.get();
}

int main() {
    std::cout << "���߳�SocketͨѶ���Գ���" << std::endl;
    std::cout << "��ѡ�����ģʽ��" << std::endl;
    std::cout << "1. ����������" << std::endl;
    std::cout << "2. �ͻ��˲���" << std::endl;
    std::cout << "3. ��ͻ��˲���" << std::endl;
    std::cout << "������ѡ�� (1-3): ";
    
    int choice;
    std::cin >> choice;
    std::cin.ignore(); // ������뻺����
    
    switch (choice) {
        case 1:
            TestServer();
            break;
        case 2:
            TestClient();
            break;
        case 3:
            TestMultipleClients();
            break;
        default:
            std::cout << "��Чѡ��" << std::endl;
            break;
    }
    
    return 0;
}

// Windows下g++编译命令示例：
// g++ main.cpp mulThreadServer.cpp mulThreadClient.cpp mulThreadClient.h mulThreadServer.h -o socket_demo.exe -lws2_32 -lwsock32 -std=c++11