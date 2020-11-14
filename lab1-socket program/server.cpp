#include <stdio.h>  
#include <winsock2.h>  
#include<iostream>
#pragma comment(lib,"ws2_32.lib")  
using namespace std;  
int main(int argc, char* argv[])  
{  
    //��ʼ��WSA  
    WORD sockVersion = MAKEWORD(2,2);  
    WSADATA wsaData;  
    if(WSAStartup(sockVersion, &wsaData)!=0)  
    {  
        return 0;  
    }  
  
    //�����׽���  
    SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
    if(slisten == INVALID_SOCKET)  
    {  
        printf("socket error !");  
        return 0;  
    }  
  
    //��IP��ַ�Ͷ˿�  
    sockaddr_in sin;  
    sin.sin_family = AF_INET;  
    sin.sin_port = htons(8888);  //�˿� 
	sin.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY 0.0.0.0 �κα���ip��ַ ����Ӧ����������� 
    if(bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)  
    {  
        printf("bind error !");  
    }  
  
    //��ʼ����  
    if(listen(slisten, 5) == SOCKET_ERROR)  
    {  
        printf("listen error !");  
        return 0;  
    }  
  
    //ѭ����������  
    SOCKET sClient;  
    sockaddr_in remoteAddr;  //׼������Զ�̵�ַ 
    int nAddrlen = sizeof(remoteAddr);  
    char revData[255];   
    while (true)  
    {  
        printf("\n�ȴ�����...\n");  
        sClient = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);  
        if(sClient == INVALID_SOCKET)  
        {  
            printf("accept error !");  
            continue;  
        }  
        printf("���ܵ����ӣ�%s \r\n", inet_ntoa(remoteAddr.sin_addr)); //inet_ntoaת����ַ��ʽ 
          
        //��������  
        int ret = recv(sClient, revData, 255, 0);         
        if(ret > 0)  
        {  
            revData[ret] = 0x00;  //����λ 
            cout<<("C:");
            string r(revData); 
            if(r=="exit")
            {
             closesocket(slisten);  
   	 		 WSACleanup();  
    		 return 0;	
			}
            printf(revData,"\n");  //��ӡ���� 
        }  
  
        //�������� 
		string data;
		cout<<("\nS:");
		getline( cin, data, '\n' );
		if(data=="exit")
		{
			
		const char * sendData;
		sendData = data.c_str();   
		 send(sClient, sendData, strlen(sendData), 0); 
			closesocket(sClient); 
				WSACleanup();
				return 0;
		}
        const char * sendData = data.c_str();  
        send(sClient, sendData, strlen(sendData), 0); 
         
    }  
      
    closesocket(slisten);  
    WSACleanup();  
    return 0;  
} 
