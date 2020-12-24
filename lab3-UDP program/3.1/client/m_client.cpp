#include <iostream>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <time.h>
#include <string>
#include <bitset>
#include <queue>
#include <cstdlib>
#include <ctime>

#pragma comment(lib, "Ws2_32.lib")
using namespace std;

const int MAXLEN = 509;
char buffer[100000000];
const unsigned char ACK = 0x01;
const unsigned char NAK = 0x02;
const unsigned char FIRST_SHAKE = 0x03;
const unsigned char SECOND_SHAKE = 0x04;
const unsigned char THIRD_SHAKE = 0x05;
const unsigned char FIRST_WAVE = 0x06;
const unsigned char SECOND_WAVE = 0x07;
const unsigned char LAST = 0x08;
const unsigned char NOTLAST = 0x18;
const int TIMEOUT = 500;
double cwnd = MAXLEN;
int ssthresh=16*MAXLEN;
int dupACK=0;

SOCKET socketClient;
SOCKADDR_IN addr_server;
SOCKADDR_IN addr_client;

unsigned char checksum(char *flag,int len){
    unsigned int sum = 0;
    unsigned char ret;
    int i=0;
	while (len--)
	{
        sum += (unsigned char)(flag[i++]);
    }

    while (sum >> 8)
	{
        sum = (sum >> 8) + (sum & 0x00ff);
    }
    ret = sum;
    return ~ret;
}

bool send(char* message,int len,int seq,int last=0){
	if(len > MAXLEN || (last == false && len != MAXLEN))
	{
		return false;
	}
	char *send_buffer;
	int send_buffer_len;
	if(!last)
	{
		send_buffer = new char[len + 3];
		send_buffer[1] = NOTLAST;
		send_buffer[2] = seq;
		for (int i = 3; i < len + 3; i++)
		{
			send_buffer[i] = message[i - 3];
		}
        send_buffer[0] = checksum(send_buffer + 1, len + 2);
        send_buffer_len = len + 3;
	}
	else
	{
		send_buffer = new char[len + 4];
		send_buffer[1] = LAST;
		send_buffer[2] = seq;
		send_buffer[3] = len;
		for(int i = 4;i<len+4;i++){
			send_buffer[i] = message[i - 4];
		}
		send_buffer[0] = checksum(send_buffer + 1 ,len + 3);
		send_buffer_len = len + 4;
	}
	sendto(socketClient, send_buffer, send_buffer_len, 0, (sockaddr *) &addr_server, sizeof(addr_server));
	return true;
}

void send_window(char* message, int len) {
	queue<pair<int,int>> Window;
	static int base = 1;	//当前滑动窗最左边的序号，从1开始
	int seq = base;			//下一个可以进入窗口的包的序号
	int num = len / MAXLEN + (len % MAXLEN != 0);
	int temp_windowlast = 0;//窗口最右端
	int temp_last = 0; 		//已经确定的最后一个
	bool itw[256] = { 0 };
	int last_pack = 0;
	int addr_len = sizeof(addr_client);
	while (1) {
		if (temp_last== num)
			break;

		if (cwnd < ssthresh && dupACK < 3)
		{
			if (Window.size() * MAXLEN < cwnd && temp_windowlast < num)
			{
				send(message + temp_windowlast * MAXLEN, temp_windowlast == num - 1 ? len % MAXLEN : MAXLEN, seq % 256, temp_windowlast == num - 1);
				Window.push(make_pair(clock(), seq % 256));
				itw[seq % 256] = 1;
				seq++;
				temp_windowlast++;
			}
			char recv[3];
			int recvsize = recvfrom(socketClient, recv, 3, 0, (sockaddr*)&addr_server, &addr_len);
			if (recvsize && checksum(recv, 3) == 0 && recv[1] == ACK && itw[(unsigned char)recv[2]]) {
				while (Window.front().second != (unsigned char)recv[2]) {
					base++;
					temp_last++;
					itw[Window.front().second] = 0;
					Window.pop();
				}
				base++;
				temp_last++;
				itw[Window.front().second] = 0;
				Window.pop();
				cwnd += MAXLEN;
				dupACK = 0;
			}
			else {
				if (last_pack==(unsigned char)recv[2]) {
					dupACK++;
					if (dupACK == 3) {
						ssthresh = cwnd / 2;
						cwnd = ssthresh + 3 * MAXLEN;
						seq = base;
						temp_windowlast -= Window.size();
						while (Window.size() != 0)
							Window.pop();
						
					}
					last_pack = (unsigned char)recv[2];
				}
					if (clock() - Window.front().first > TIMEOUT) {
						seq = base;
						temp_windowlast -= Window.size();
						while (Window.size() != 0)
							Window.pop();
						ssthresh = cwnd / 2;
						cwnd = MAXLEN;
						dupACK = 0;
					}
			}
				//cout << "slow start  " << dupACK << " " << cwnd / MAXLEN << endl;
				//cout << "文件已接收" << temp_last << "个包, 完成了"<<(double)temp_last/(double)num << endl;
		}
		//拥塞避免阶段
		else if (cwnd >= ssthresh && dupACK<3) {
			if (Window.size()*MAXLEN < cwnd && temp_windowlast < num) {
				send(message + temp_windowlast * MAXLEN, temp_windowlast == num - 1 ? len % MAXLEN : MAXLEN, seq % 256, temp_windowlast == num - 1);
				Window.push(make_pair(clock(), seq % 256));
				itw[seq % 256] = 1;
				seq++;
				temp_windowlast++;
			}
			char recv[3];
			bool recvsec = recvfrom(socketClient, recv, 3, 0, (sockaddr*)&addr_server,&addr_len);
			if (recvsec && checksum(recv, 3) == 0 && recv[1] == ACK && itw[(unsigned char)recv[2]]) {
				while (Window.front().second != (unsigned char)recv[2]) {
					base++;
					temp_last++;
					itw[Window.front().second] = 0;
					Window.pop();
				}
				base++;
				temp_last++;
				itw[Window.front().second] = 0;
				Window.pop();
				cwnd += MAXLEN * (MAXLEN / cwnd);
				dupACK = 0;
			}
			else {
				if (last_pack==(unsigned char)recv[2]) {
					dupACK++;
					if (dupACK == 3) {
						ssthresh = cwnd / 2;
						cwnd = ssthresh + 3 * MAXLEN;
						seq = base;
						temp_windowlast -= Window.size();
						while (Window.size() != 0)
							Window.pop();
					}
					last_pack = (unsigned char)recv[2];
				}
					
				if (clock() - Window.front().first > TIMEOUT) {
					seq = base;
					temp_windowlast -= Window.size();
					while (Window.size() != 0)
						Window.pop();
					ssthresh = cwnd / 2;
					cwnd = MAXLEN;
					dupACK = 0;
				}
			}
	
			//cout << "congestion avoidance  " << dupACK <<"  "<<cwnd<< endl;
		}
		//快速恢复阶段
		else if (dupACK==3) {
			
			if (Window.size() * MAXLEN < cwnd && temp_windowlast < num) {
				send(message + temp_windowlast * MAXLEN, temp_windowlast == num - 1 ? len % MAXLEN : MAXLEN, seq % 256, temp_windowlast == num - 1);
				Window.push(make_pair(clock(), seq % 256));
				itw[seq % 256] = 1;
				seq++;
				temp_windowlast++;
			}
			char recv[3];
			bool recvsec = recvfrom(socketClient, recv, 3, 0, (sockaddr*)&addr_server, &addr_len);
			if (recvsec && checksum(recv, 3) == 0 && recv[1] == ACK && itw[(unsigned char)recv[2]]) {
				while (Window.front().second != (unsigned char)recv[2]) {
					base++;
					temp_last++;
					itw[Window.front().second] = 0;
					Window.pop();
				}
				base++;
				temp_last++;
				itw[Window.front().second] = 0;
				Window.pop();
				cwnd = ssthresh;
				dupACK = 0;
			}
			else {
				if (last_pack == (unsigned char)recv[2]) {
					cwnd += MAXLEN;
					last_pack = (unsigned char)recv[2];
				}
				if (clock() - Window.front().first > TIMEOUT) {
					seq = base;
					temp_windowlast -= Window.size();
					while (Window.size() != 0)
						Window.pop();
					ssthresh = cwnd / 2;
					cwnd = MAXLEN;
					dupACK = 0;
				}
			}
			//cout << "quick recovery  " <<dupACK<<"  "<< cwnd<<endl;
		}

	}
}

int main(){
	
	WSADATA wsaData;
	WORD wdRquVersion = MAKEWORD(2,2);
	int nRes = WSAStartup(wdRquVersion,&wsaData);
	if(nRes != 0)
	{
		int a = WSAGetLastError();
		printf("网络库打开失败\n", a);
        system("pause");
        return 0;
	}

	if (HIBYTE(wsaData.wVersion) != 2 || LOBYTE(wsaData.wVersion) != 2)
	{
		printf("打开的网络库与需求不符\n");
		WSACleanup();
		system("pause");
		return 0;
	}

	socketClient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (INVALID_SOCKET == socketClient)
	{
		int a = WSAGetLastError();
		printf("网络库打开失败 %d\n", a);
		WSACleanup();
		system("pasue");
		return 0;
	}

    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(8888);
    addr_server.sin_addr.s_addr = inet_addr("127.0.0.1");

    int time_out = 1;
    setsockopt(socketClient, SOL_SOCKET, SO_RCVTIMEO, (char *)&time_out, sizeof(time_out));
	
	printf("准备连接！\n");
	while(1)
	{
		char shake[2];
		shake[1] = FIRST_SHAKE;
		shake[0] = checksum(shake + 1,1);
		sendto(socketClient, shake, sizeof(shake), 0, (SOCKADDR*) &addr_server, sizeof(addr_server));
		
		int begin = clock();

		char shake_recv[2];
		int  fail = 0;
		int  len_client_shake = sizeof(addr_client);
		while(SOCKET_ERROR == recvfrom(socketClient, shake_recv, sizeof(shake_recv), 0, (SOCKADDR*) &addr_server, &len_client_shake))
		{
			if (clock() - begin > TIMEOUT)
			{
                fail = 1;
                break;
            }
		}
		if(fail == 0 && checksum(shake_recv,2) == 0 && shake_recv[1] == SECOND_SHAKE)
		{
			shake[1] = THIRD_SHAKE;
			shake[0] = checksum(shake + 1,1);
			sendto(socketClient, shake, sizeof(shake), 0, (SOCKADDR*) &addr_server, sizeof(addr_server));
            break;
		}
	}
	printf("连接成功！\n");

	while(1)
	{
		string filename;
		int len = 0;
		printf("请输入发送的文件名(如果需要退出，请输入exit)："); 
		cin>>filename;
		if(!strcmp("exit",filename.c_str()))
		{
			send_window((char*)(filename.c_str()), filename.length());
			break;
		}
		ifstream in(filename.c_str(),ifstream::binary);
		if(!in)
		{
			printf("文件不存在，请输入正确的文件名：\n");
			continue;
		}
		else
		{
			printf("文件打开成功！\n");
		}
		unsigned char t = in.get();
		while(in)
		{
			buffer[len++] = t;
			t = in.get();
		}
		in.close();
		send_window((char*)(filename.c_str()), filename.length());
		int begintime = clock();
		send_window(buffer, len);
		int endtime = clock();
		memset(buffer, 0, sizeof(buffer) / sizeof(char));
		int runtime = (endtime-begintime)*1000/CLOCKS_PER_SEC;
		printf("文件传输成功！");
		printf("传输时间为：%d ms\n",runtime);
	}

	while(1)
	{
		char wave[2];
		wave[1] = FIRST_WAVE;
		wave[0] = checksum(wave + 1,1);
		sendto(socketClient, wave, sizeof(wave), 0, (SOCKADDR *) &addr_server, sizeof(addr_server));

		int begin = clock();
		char wave_recv[2];
		int len_client_wave = sizeof(addr_client);
		int fail = 0;
		while(SOCKET_ERROR == recvfrom(socketClient, wave_recv, sizeof(wave_recv), 0, (SOCKADDR *) &addr_server, &len_client_wave))
		{
			if (clock() - begin > TIMEOUT)
			{
            	fail = 1;
            	break;
       		}
		}
		if(fail == 0 && checksum(wave_recv,2) == 0 && wave_recv[1] == SECOND_WAVE)
		{
            break;
		}
	}
	printf("断开连接！\n");

	closesocket(socketClient);
    WSACleanup();
	system("pause");
    return 0;
}
