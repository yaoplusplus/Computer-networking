#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <winsock2.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include<time.h>
#include <thread>

using namespace std;
const int MAXLEN = 509;
char buffer[200000000];

const unsigned char ACK = 0x01;
const unsigned char NAK = 0x02;
const unsigned char LAST = 0x08;
const unsigned char NOTLAST = 0x18;
const unsigned char FSHAKE = 0x03;
const unsigned char SSHAKE = 0x04;
const unsigned char TSHAKE = 0x05;
const unsigned char FWAKE = 0x06;
const unsigned char SWAKE = 0x07;
const int MAX_WAIT_TIME = 500;

SOCKET server;
SOCKADDR_IN serverAddr,clientAddr;
//计算校验和
unsigned char checksum(char *package,int len){
	if (len == 0){
		return ~(0);
	}
    unsigned int sum = 0;
    int i = 0;
    while(len--){
        sum += (unsigned char) package[i++];
        if(sum & 0xFF00){
            sum &= 0x00FF;
            sum++;
        } 
    }
    return ~(sum&0x00FF);
}

void ARQ_rev(char *pkt,int &len_recv){
	char recv[MAXLEN + 4];
    int len_tmp = sizeof(clientAddr);
    static char last_order = -1;//上一个包的序号
    
    len_recv = 0;
    while (true) {
        while (true) {
            memset(recv,0,sizeof(recv));
            while (recvfrom(server, recv, MAXLEN + 4, 0, (sockaddr *) &clientAddr, &len_tmp) == SOCKET_ERROR);
            char send[3];
            if (checksum(recv, MAXLEN + 4) == 0) {
                send[1] = ACK;
                send[2] = recv[2];
                send[0] = checksum(send + 1, 2);
                sendto(server, send, 3, 0, (sockaddr *) &clientAddr, sizeof(clientAddr));
                break;
            } else {
                send[1] = NAK;
                send[2] = recv[2];
                send[0] = checksum(send + 1, 2);
                sendto(server, send, 3, 0, (sockaddr *) &clientAddr, sizeof(clientAddr));
                cout << "NAK" << endl;
                continue;
            }
        }
        if (last_order == recv[2])
        // 再次收到了上一个包 再次收包 所以会多次给客户端发一个同一个ack
            continue;
        // 更新服务端收到包序号
        last_order = recv[2];
        // 解包
        if (LAST == recv[1]) {
            cout<<"最后一个包的长度: "<<int(recv[3])<<endl;
            for (int i = 4; i < recv[3] + 4; i++)
                pkt[len_recv++] = recv[i];
            break;
        } else {
            for (int i = 3; i < MAXLEN + 3; i++)
                pkt[len_recv++] = recv[i];
        }
    }
}

int main(){
	WSADATA wsadata;
    if(WSAStartup(MAKEWORD(2,2),&wsadata)){
		printf("error");
		return 0;
	}

    serverAddr.sin_family = AF_INET; //使用ipv4
    serverAddr.sin_port = htons(5799); //端口
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    server = socket(AF_INET, SOCK_DGRAM, 0);

    int time_out=1;//1ms超时
    setsockopt(server, SOL_SOCKET, SO_RCVTIMEO, (char *)&time_out, sizeof(time_out));

    if (server == INVALID_SOCKET) {
        printf("socket of server invalid!");
        closesocket(server);
        return 0;
    }
    if(bind(server, (sockaddr *) (&serverAddr), sizeof(serverAddr))==SOCKET_ERROR){
    	printf("bind fail");
        closesocket(server);
        WSACleanup();
        return 0;
	}
	printf("\n----------等待连接----------\n");
    //开始握手
	while(true){
		char recv[2];
		int len_tmp = sizeof(clientAddr);
		while (recvfrom(server, recv, 2, 0, (sockaddr *) &clientAddr, &len_tmp) == SOCKET_ERROR);
		if(checksum(recv,2)!=0 || recv[1] != FSHAKE){
			continue;
		}
		while(true){
			recv[1] = SSHAKE;
			recv[0] = checksum(recv + 1,1);
			sendto(server, recv, 2, 0, (sockaddr *) &clientAddr, sizeof(clientAddr));
            while (recvfrom(server, recv, 2, 0, (sockaddr *) &clientAddr, &len_tmp) == SOCKET_ERROR);
            if (checksum(recv, 2) == 0 && recv[1] == FSHAKE)
                continue;
            if (checksum(recv, 2) == 0 && recv[1] == TSHAKE)
                break;
            if (checksum(recv, 2) != 0 || recv[1] != TSHAKE) {
                printf("error");
                return 0;
            }
		}
		break;
        
	}
    printf("----------成功连接----------\n");

    while(true){
    // 接受文件名
	int len = 0;
	ARQ_rev(buffer,len);
	buffer[len] = 0;
	string file_name(buffer);
    if(!strcmp("exit",file_name.c_str())){
        break;
    }
    // 重置buffer
    memset(buffer,0,file_name.length());
    // 接受文件内容
	ARQ_rev(buffer,len);
	ofstream out(file_name.c_str(),ofstream::binary);
	for(int i = 0;i<len;i++){
		out<<buffer[i];
	}
	out.close();
    printf("收到文件: %s\n",file_name.c_str());
    // cout<<"file_name: "<<file_name<<endl;
    }

	while(true){
		char recv[2];
        int len_tmp = sizeof(clientAddr);
        while (recvfrom(server, recv, 2, 0, (sockaddr *) &clientAddr, &len_tmp) == SOCKET_ERROR);
        if (checksum(recv, 2) != 0 || recv[1] != (char)FWAKE)
            continue;

        recv[1] = SWAKE;
        recv[0] = checksum(recv + 1, 1);
        sendto(server, recv, 2, 0, (sockaddr *) &clientAddr, sizeof(clientAddr));
        break;
    }
	printf("\n----------断开连接----------");
	system("pause"); 
	return 0;
}
