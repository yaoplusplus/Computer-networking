#include "..\common.h"

SOCKET server;
SOCKADDR_IN serverAddr;
SOCKADDR_IN clientAddr;

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

void recv(char *message,int &len_recv){
	char recv[MAXLEN + 4];
    int len_tmp = sizeof(clientAddr);
    static char last_seqnum = 0;
    len_recv = 0;
    while (true) 
    {
        while (true) 
        {
            memset(recv,0,sizeof(recv));
            while (recvfrom(server, recv, MAXLEN + 4, 0, (sockaddr *) &clientAddr, &len_tmp) == SOCKET_ERROR);
            char send[3];
            if (checksum(recv, MAXLEN + 4) == 0) 
            {
                send[1] = ACK;
                send[2] = recv[2];
                send[0] = checksum(send + 1, 2);
                sendto(server, send, 3, 0, (sockaddr *) &clientAddr, sizeof(clientAddr));
                break;
            } 
            else 
            {
                send[1] = NAK;
                send[2] = recv[2];
                send[0] = checksum(send + 1, 2);
                sendto(server, send, 3, 0, (sockaddr *) &clientAddr, sizeof(clientAddr));
                cout << "NAK" << endl;
                continue;
            }
        }
        if (last_seqnum == recv[2])
            continue;
        last_seqnum = recv[2];
        if (LAST == recv[1]) 
        {
            for (int i = 4; i < strlen(recv); i++)
                message[len_recv++] = recv[i];
            break;
        } 
        else 
        {
            for (int i = 3; i < MAXLEN + 3; i++)
                message[len_recv++] = recv[i];
        }
    }
}

int main(){

    WSADATA wsaData;
    WORD wdRquVersion = MAKEWORD(2,2);
    WSAStartup(wdRquVersion,&wsaData);
    server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    serverAddr.sin_family = AF_INET; 
    serverAddr.sin_port = htons(8888);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (SOCKET_ERROR == bind(server, (SOCKADDR*)& serverAddr, sizeof(serverAddr)))
	{
		int a = WSAGetLastError();
		printf("bind error %d\n", a);
		closesocket(server);
		WSACleanup();
		return 0;
	}

    int time_out=1;
    setsockopt(server, SOL_SOCKET, SO_RCVTIMEO, (char *)&time_out, sizeof(time_out));

	printf("\n----------开始连接！----------\n");
	while(1)
    {
		char shake_recv[2];
		int  len_client_shake = sizeof(clientAddr);
		while (SOCKET_ERROR == recvfrom(server, shake_recv, sizeof(shake_recv), 0, (sockaddr *) &clientAddr, &len_client_shake));
		if(checksum(shake_recv,2)!=0 || shake_recv[1] != FIRST_SHAKE)
        {
			continue;
		}
		while(1)
        {
            char shake_recv[2];
			shake_recv[1] = SECOND_SHAKE;
			shake_recv[0] = checksum(shake_recv + 1,1);
			sendto(server, shake_recv, sizeof(shake_recv), 0, (sockaddr *) &clientAddr, sizeof(clientAddr));
            while (SOCKET_ERROR == recvfrom(server, shake_recv, sizeof(shake_recv), 0, (sockaddr *) &clientAddr, &len_client_shake));
            if (checksum(shake_recv, 2) == 0 && shake_recv[1] == FIRST_SHAKE)
            {
                continue;
            }
            if (checksum(shake_recv, 2) == 0 && shake_recv[1] == THIRD_SHAKE)
            {   
                printf("----------连接成功！----------\n\n");
                break;
            }
            if (checksum(shake_recv, 2) != 0 || shake_recv[1] != THIRD_SHAKE)
            {
                printf("----------连接失败！----------\n\n");
                return 0;
            }
		}
		break;
	}
    
    while(1)
	{
        int len = 0;
        ZeroMemory(buffer,sizeof(buffer));
	    recv(buffer,len);
	    buffer[len] = 0;
	    string file_name(buffer);
        if(!strcmp("exit",file_name.c_str()))
        {
            break;
        }
        ZeroMemory(buffer,len);
	    recv(buffer,len);
	    ofstream out(file_name.c_str(),ofstream::binary);
	    for(int i = 0;i<len;i++)
        {
		    out<<buffer[i];
	    }
	    out.close();
        printf("文件 %s 传输成功！\n",file_name.c_str());
    }

	while(1)
    {
		char wave_recv[2];
        int len_client_wave = sizeof(clientAddr);
        while (SOCKET_ERROR == recvfrom(server, wave_recv, sizeof(wave_recv), 0, (SOCKADDR *) &clientAddr, &len_client_wave));
        if (checksum(wave_recv, 2) != 0 || wave_recv[1] != FIRST_WAVE)
        {    
            continue;
        }
        wave_recv[1] = SECOND_WAVE;
        wave_recv[0] = checksum(wave_recv + 1, 1);
        sendto(server, wave_recv, sizeof(wave_recv), 0, (SOCKADDR *) &clientAddr, sizeof(clientAddr));
        break;
	}
	printf("\n----------断开连接！----------\n");

    closesocket(server);
    WSACleanup();
	system("pause"); 
	return 0;
}
