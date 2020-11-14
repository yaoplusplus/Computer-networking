#include <stdio.h>  
#include <winsock2.h>  
#include<iostream>
#pragma comment(lib,"ws2_32.lib")  
using namespace std;  
int main(int argc, char* argv[])  
{  
    //初始化WSA  
    WORD sockVersion = MAKEWORD(2,2);  
    WSADATA wsaData;  
    if(WSAStartup(sockVersion, &wsaData)!=0)  
    {  
        return 0;  
    }  
  
    //创建套接字  
    SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
    if(slisten == INVALID_SOCKET)  
    {  
        printf("socket error !");  
        return 0;  
    }  
  
    //绑定IP地址和端口  
    sockaddr_in sin;  
    sin.sin_family = AF_INET;  
    sin.sin_port = htons(8888);  //端口 
	sin.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY 0.0.0.0 任何本机ip地址 （对应多网卡情况） 
    if(bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)  
    {  
        printf("bind error !");  
    }  
  
    //开始监听  
    if(listen(slisten, 5) == SOCKET_ERROR)  
    {  
        printf("listen error !");  
        return 0;  
    }  
  
    //循环接收数据  
    SOCKET sClient;  
    sockaddr_in remoteAddr;  //准备接收远程地址 
    int nAddrlen = sizeof(remoteAddr);  
    char revData[255];   
    while (true)  
    {  
        printf("\n等待连接...\n");  
        sClient = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);  
        if(sClient == INVALID_SOCKET)  
        {  
            printf("accept error !");  
            continue;  
        }  
        printf("接受到连接：%s \r\n", inet_ntoa(remoteAddr.sin_addr)); //inet_ntoa转换地址形式 
          
        //接收数据  
        int ret = recv(sClient, revData, 255, 0);         
        if(ret > 0)  
        {  
            revData[ret] = 0x00;  //置零位 
            cout<<("C:");
            string r(revData); 
            if(r=="exit")
            {
             closesocket(slisten);  
   	 		 WSACleanup();  
    		 return 0;	
			}
            printf(revData,"\n");  //打印数据 
        }  
  
        //发送数据 
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
