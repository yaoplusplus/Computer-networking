#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <winsock2.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <time.h>
#include <thread>

using namespace std;
const int MAXLEN = 509;
char buffer[200000000];
const unsigned char ACK = 0x01;
const unsigned char NAK = 0x02;
const unsigned char LAST = 0x08; //0000 0100
const unsigned char NOTLAST = 0x18;
const unsigned char FSHAKE = 0x03;
const unsigned char SSHAKE = 0x04;
const unsigned char TSHAKE = 0x05;
const unsigned char WAVE_1 = 0x06;
const unsigned char WAVE_2 = 0x07;
const int TIMEOUT = 500;
SOCKET client;
SOCKADDR_IN serverAddr,clientAddr;
//计算校验和
unsigned char sum(char *flag,int len){
	if (len == 0){
		return ~(0);
	}
    unsigned int ret = 0;
    int i = 0;
    while(len--){
        ret += (unsigned char) flag[i++];
        if(ret & 0xFF00){
            ret &= 0x00FF;
            ret++;
        } 
    }
    return ~(ret&0x00FF);
}

bool my_send(char* message,int len,int order,int last=0){
	if(len > MAXLEN || (last == false && len != MAXLEN)){
		return false;
	}
	char *tmp;
	int tmp_len;
	if(!last){//make package
		tmp = new char[len + 3];
		tmp[1] = NOTLAST;
		tmp[2] = order;
		for (int i = 3; i < len + 3; i++)
		{
			tmp[i] = message[i - 3];
		}
        tmp[0] = sum(tmp + 1, len + 2);
        tmp_len = len + 3;
	}else{
		
		tmp = new char[len + 4];
		tmp[1] = LAST;
		tmp[2] = order;
		tmp[3] = len;
		for(int i = 4;i<len+4;i++){
			tmp[i] = message[i - 4];
		}
		tmp[0] = sum(tmp + 1 ,len + 3);
		tmp_len = len + 4;

		
		
	}
	//send package
	while(true){
		sendto(client, tmp, tmp_len, 0, (sockaddr *) &serverAddr, sizeof(serverAddr));
		int begin = clock();
		char recv[3];
		int len_tmp = sizeof(serverAddr);
		int fail = 0;
		
		while (recvfrom(client, recv, 3, 0, (sockaddr *) &serverAddr, &len_tmp) == SOCKET_ERROR){    
			if (clock() - begin > TIMEOUT) {
                fail = 1;
                break;
            }
        }
        if (fail == 0 && sum(recv, 3) == 0 && recv[1] == ACK && recv[2] == (char)order)
            return true;
	}
}

int main(){
	WSADATA wsadata;
	//package的序号
	int order = 0;
	if(WSAStartup(MAKEWORD(2,2),&wsadata)){
		printf("error");
		return 0;
	}

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5799);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    client = socket(AF_INET, SOCK_DGRAM, 0);

    int time_out = 1;
    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char *)&time_out, sizeof(time_out));
    
	if(client == INVALID_SOCKET){
    	printf("socket of client invalid!");
		closesocket(client);
    	return 0;
	}
	// 建立连接
	printf("\n----------开始连接----------\n");
	while(true){
		//package[0]为检验和
		char shake_package[2];
		shake_package[1] = FSHAKE;
		shake_package[0] = sum(shake_package + 1,1);
		sendto(client, shake_package, 2, 0, (sockaddr *) &serverAddr, sizeof(serverAddr));
		int begin = clock();
		char recv[2];
		int len = sizeof(clientAddr);
		int fail = 0;
		while(recvfrom(client, recv, 2, 0, (sockaddr *) &serverAddr, &len) == SOCKET_ERROR){
			if (clock() - begin > TIMEOUT) {
                fail = 1;
                break;
            }
		}
		if(fail == 0 && sum(recv,2) == 0 && recv[1] == SSHAKE)
		{
			shake_package[1] = TSHAKE;
			shake_package[0] = sum(shake_package + 1,1);
			sendto(client, shake_package, 2, 0, (sockaddr *) &serverAddr, sizeof(serverAddr));
            break;
		}
	}
	printf("----------成功连接----------\n\n");

	while(true){
		// 输入欲发送文件名
		string filename;
		printf("欲发送文件名:"); 
		cin>>filename;
		if(!strcmp("exit",filename.c_str())){
			my_send("exit",filename.length(),order++,1);
			break;}
		// 使用二进制方式 打开当前目录下的文件
		ifstream in(filename.c_str(),ifstream::binary);
		int len = 0;
		if(!in){
			printf("can't open the file!\n请再次输入");
			continue;
		}
		// 文件读取到buffer
		BYTE t = in.get();
		while(in){
			buffer[len++] = t;
			t = in.get();
		}
		in.close();
		printf("文件加载完\n");
		// 发送文件名
		my_send((char *) (filename.c_str()),filename.length(),order++,1);
		printf("文件名发送完\n");
		// 发送文件
		//TODO what do this mean ?
		order %= (1<<8);
		int num = len / MAXLEN + (len % MAXLEN != 0);
		for(int i = 0;i<num;i++){
			int temp;
			int ttemp;
			if(i == num - 1)
			{
				ttemp = 1;
				temp = len - (num - 1)*MAXLEN;
			}
			else{
				ttemp = 0;
				temp = MAXLEN;
			}
			my_send(buffer + i * MAXLEN,temp,order++,ttemp);
			order %= (1<<8); 
	}
	}
	
	printf("\n----------断开连接----------");
	while(true){
		char tmp[2];
		tmp[1] = WAVE_1;
		tmp[0] = sum(tmp + 1,1);
		sendto(client, tmp, 2, 0, (sockaddr *) &serverAddr, sizeof(serverAddr));
		int begin = clock();
		char recv[2];
		int len = sizeof(clientAddr);
		int fail = 0;
		while(recvfrom(client, recv, 2, 0, (sockaddr *) &serverAddr, &len) == SOCKET_ERROR){
			if (clock() - begin > TIMEOUT) {
                fail = 1;
                break;
            }
		}
		if(fail == 0 && sum(recv,2) == 0 && recv[1] == WAVE_2)
		{
            break;
		}
	}    
	closesocket(client);
    WSACleanup();
	system("pause");
    return 0;
}















