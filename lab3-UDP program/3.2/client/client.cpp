#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <winsock2.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <time.h>
#include <thread>
#include "window.h"
#define WINDOW_LEN 10 //���崰�ڳ��� ������: sendbase+WINDOW_LEN-1
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
const unsigned char FWAKE = 0x06;
const unsigned char SWAKE = 0x07;
const int MAX_WAIT_TIME = 500;


SOCKET client;

SOCKADDR_IN serverAddr,clientAddr;
slide_window sw;
//����У���
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

bool ARQ_send(char* pkt,int len,int serial_num,int last=0){
	if(len > MAXLEN || (last == false && len != MAXLEN)){
		return false;
	}
	char *real_package;//����3/4λchar:checksum,LAST_FLAG,num(��ѡ)
	int tmp_len;
	if(!last){//make package
		real_package = new char[len + 3];
		real_package[1] = NOTLAST;
		real_package[2] = serial_num;
		for (int i = 3; i < len + 3; i++)
		{
			real_package[i] = pkt[i - 3];
		}
        real_package[0] = checksum(real_package + 1, len + 2);
        tmp_len = len + 3;
		sw.slide_w[serial_num].pkt = real_package; //���͵İ��Ž�������
	}else{
		real_package = new char[len + 4];
		real_package[1] = LAST;
		real_package[2] = serial_num;
		real_package[3] = len;
		for(int i = 4;i<len+4;i++){
			real_package[i] = pkt[i - 4];
		}
		real_package[0] = checksum(real_package + 1 ,len + 3);
		tmp_len = len + 4;
		sw.slide_w[serial_num].pkt = real_package; //���͵İ��Ž�������
	}
	//send package
	while(true){
		sendto(client, real_package, tmp_len, 0, (sockaddr *) &serverAddr, sizeof(serverAddr));
		int begin = clock();
		char recv[3];
		int len_tmp = sizeof(serverAddr);
		int fail = 0;
		
		while (recvfrom(client, recv, 3, 0, (sockaddr *) &serverAddr, &len_tmp) == SOCKET_ERROR){    
			if (clock() - begin > MAX_WAIT_TIME) {
                fail = 1;
                break;
            }
        }
        if (fail == 0 && checksum(recv, 3) == 0 && recv[1] == ACK && recv[2] == (char)serial_num)
            return true;
	}
}

void rev_check(){
cout<<"\n----------����ACK�߳�����----------\n";
char rev_ack[4];
while(recvfrom());
}


int main(){
	WSADATA wsadata;
	//package�����
	int serial_num = 0;
	if(WSAStartup(MAKEWORD(2,2),&wsadata)){
		printf("�汾����!");
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
	// ��������
	printf("\n----------��ʼ����----------\n");
	while(true){
		//package[0]Ϊ�����
		char shake_package[2];
		shake_package[1] = FSHAKE;
		shake_package[0] = checksum(shake_package + 1,1);
		sendto(client, shake_package, 2, 0, (sockaddr *) &serverAddr, sizeof(serverAddr));
		int begin = clock();
		char recv[2];
		int len = sizeof(clientAddr);
		int fail = 0;
		while(recvfrom(client, recv, 2, 0, (sockaddr *) &serverAddr, &len) == SOCKET_ERROR){
			if (clock() - begin > MAX_WAIT_TIME) {
                fail = 1;
                break;
            }
		}
		if(fail == 0 && checksum(recv,2) == 0 && recv[1] == SSHAKE)
		{
			shake_package[1] = TSHAKE;
			shake_package[0] = checksum(shake_package + 1,1);
			sendto(client, shake_package, 2, 0, (sockaddr *) &serverAddr, sizeof(serverAddr));
            break;
		}
	}
	printf("----------�ɹ�����----------\n\n");
	thread rev(rev_check);
	// rev.join();
	while(true){
		// �����������ļ���
		string filename;
		printf("�������ļ���:"); 
		cin>>filename;
		if(!strcmp("exit",filename.c_str())){
			ARQ_send((char*)filename.c_str(),filename.length(),serial_num++,1);
			break;}
		// ʹ�ö����Ʒ�ʽ �򿪵�ǰĿ¼�µ��ļ�
		ifstream in(filename.c_str(),ifstream::binary);
		int len = 0;
		if(!in){
			printf("can't open the file!\n���ٴ�����");
			continue;
		}
		// �ļ���ȡ��buffer
		BYTE t = in.get();
		while(in){
			buffer[len++] = t;
			t = in.get();
		}
		in.close();
		printf("�ļ�������\n");
		// �����ļ���
		ARQ_send((char *) (filename.c_str()),filename.length(),serial_num++,1);
		printf("�ļ���������\n");
		// �����ļ�
		// ��ֹ���к����unsigne char
		serial_num %= (1<<8);
		int num = len / MAXLEN + (len % MAXLEN != 0);
		for(int i = 0;i<num;i++){
			int len_package;
			int last_flag;
			if(i == num - 1)
			{
				last_flag = 1;
				len_package = len - (num - 1)*MAXLEN;
			}
			else{
				last_flag = 0;
				len_package = MAXLEN;
			}
			//�ü�֮��İ����뻬������
			sw.slide_w[serial_num].pkt = buffer + i * MAXLEN;
			sw.slide_w[serial_num].length = len_package;
			sw.slide_w[serial_num].last_flag = last_flag;

			ARQ_send(buffer + i * MAXLEN,len_package,serial_num++,last_flag);
			serial_num %= (1<<8); 
	}
	}
	
	printf("\n----------�Ͽ�����----------");
	while(true){
		char tmp[2];
		tmp[1] = FWAKE;
		tmp[0] = checksum(tmp + 1,1);
		sendto(client, tmp, 2, 0, (sockaddr *) &serverAddr, sizeof(serverAddr));
		int begin = clock();
		char recv[2];
		int len = sizeof(clientAddr);
		int fail = 0;
		while(recvfrom(client, recv, 2, 0, (sockaddr *) &serverAddr, &len) == SOCKET_ERROR){
			if (clock() - begin > MAX_WAIT_TIME) {
                fail = 1;
                break;
            }
		}
		if(fail == 0 && checksum(recv,2) == 0 && recv[1] == SWAKE)
		{
            break;
		}
	}    
	closesocket(client);
    WSACleanup();
	system("pause");
    return 0;
}















