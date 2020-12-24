#include "..\common.h"
using namespace std;


SOCKET client;
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

bool send(char* message,int content_len,int nextseqnum,int last_flag=0){
	if(content_len > MAXLEN || (last_flag == false && content_len != MAXLEN))
	{
		return false;
	}
	char *send_buffer;
	int send_buffer_len;
	if(!last_flag)
	{
		send_buffer = new char[content_len + 3];
		send_buffer[1] = NOTLAST;
		send_buffer[2] = nextseqnum;
		for (int i = 3; i < content_len + 3; i++)
		{
			send_buffer[i] = message[i - 3];
		}
        send_buffer[0] = checksum(send_buffer + 1, content_len + 2);
        send_buffer_len = content_len + 3;
	}
	else
	{
		send_buffer = new char[content_len + 4];
		send_buffer[1] = LAST;
		send_buffer[2] = nextseqnum;
		send_buffer[3] = content_len;
		for(int i = 4;i<content_len+4;i++){
			send_buffer[i] = message[i - 4];
		}
		send_buffer[0] = checksum(send_buffer + 1 ,content_len + 3);
		send_buffer_len = content_len + 4;
	}
	sendto(client, send_buffer, send_buffer_len, 0, (sockaddr *) &serverAddr, sizeof(serverAddr));
	return true;
}

void sw_send(char* message, int content_len) {
	// 定义滑动窗
	queue<pair<int,int>> sw;

	static int sendbase = 1;	
	int nextseqnum = sendbase;
	//需要发送的包的总数量		
	int num = content_len / MAXLEN + (content_len % MAXLEN != 0);
	//已发送的包的数量
	int sent_pktnum = 0;
	//当前包的序号
	int curpkt_num = 0; 		
	bool ackwindow[256] = { 0 };
	while (1) {
		if (curpkt_num== num)
			break;
		if(sw.size() < Window_Size && sent_pktnum < num)
		{
			send(message + sent_pktnum * MAXLEN, sent_pktnum == num - 1 ? content_len % MAXLEN : MAXLEN, nextseqnum % 256, sent_pktnum == num - 1);
			sw.push(make_pair(clock(), nextseqnum % 256));
			ackwindow[nextseqnum%256] = true;//意思是发出去了?
			nextseqnum++;
			sent_pktnum++;
		}
		char rev_ack[3];
		int addr_len = sizeof(clientAddr);
		int recvsize = recvfrom(client, rev_ack, 3, 0, (sockaddr*)&serverAddr, &addr_len);
		if (recvsize&& checksum(rev_ack,3)==0 &&rev_ack[1]==ACK && ackwindow[(unsigned char)rev_ack[2]])
		{
			while (sw.front().second != (unsigned char)rev_ack[2])
			{	//收到的ACK的序号不对
				sendbase++;
				curpkt_num++;
				//ACK置零
				ackwindow[sw.front().second] = 0;
				sw.pop();
			}
			sendbase++;
			curpkt_num++;
			ackwindow[sw.front().second] = 0;
			sw.pop();
		}
		else {
			if (clock() - sw.front().first > TIMEOUT) {
				nextseqnum = sendbase;
				sent_pktnum -= sw.size();
				while (sw.size() != 0)
					sw.pop();
			}
		}
	}
}

int main(){
	
	WSADATA wsaData;
	WORD wdRquVersion = MAKEWORD(2,2);
	WSAStartup(wdRquVersion,&wsaData);
	client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int time_out = 1;
    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char *)&time_out, sizeof(time_out));
	
	printf("\n----------开始连接----------！\n");
	while(1)
	{
		char shake[2];
		shake[1] = FIRST_SHAKE;
		shake[0] = checksum(shake + 1,1);
		sendto(client, shake, sizeof(shake), 0, (SOCKADDR*) &serverAddr, sizeof(serverAddr));
		
		int begin = clock();

		char shake_recv[2];
		int  fail = 0;
		int  len_client_shake = sizeof(clientAddr);
		while(SOCKET_ERROR == recvfrom(client, shake_recv, sizeof(shake_recv), 0, (SOCKADDR*) &serverAddr, &len_client_shake))
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
			sendto(client, shake, sizeof(shake), 0, (SOCKADDR*) &serverAddr, sizeof(serverAddr));
            break;
		}
	}
	printf("----------连接成功----------！\n\n");

	while(1)
	{
		string filename;
		int filelen = 0;
		printf("请输入欲发送文件名: "); 
		cin>>filename;
		if(!strcmp("exit",filename.c_str()))
		{
			sw_send((char*)(filename.c_str()), filename.length());
			break;
		}
		ifstream in(filename.c_str(),ifstream::binary);
		if(!in)
		{
			printf("文件不存在，");
			continue;
		}
		sw_send((char*)(filename.c_str()), filename.length());
		// cout<<"文件名发送完\n";
		BYTE t = in.get();
		while(in)
		{
			buffer[filelen++] = t;
			t = in.get();
		}
		in.close();
		// cout<<"文件加载完成\n";
		sw_send(buffer, filelen);
		cout<<"文件发送完成！\n";
		memset(buffer, 0, sizeof(buffer));
	}

	while(1)
	{
		char wave[2];
		wave[1] = FIRST_WAVE;
		wave[0] = checksum(wave + 1,1);
		sendto(client, wave, sizeof(wave), 0, (SOCKADDR *) &serverAddr, sizeof(serverAddr));

		int begin = clock();
		char wave_recv[2];
		int len_client_wave = sizeof(clientAddr);
		int fail = 0;
		while(SOCKET_ERROR == recvfrom(client, wave_recv, sizeof(wave_recv), 0, (SOCKADDR *) &serverAddr, &len_client_wave))
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
	printf("\n----------断开连接！----------\n");

	closesocket(client);
    WSACleanup();
	system("pause");
    return 0;
}
