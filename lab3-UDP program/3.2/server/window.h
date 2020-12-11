struct ackwindow{
char*pkt;//放置发送的包 发包的时候更新这个 不考虑握手和挥手 
bool ACK;
bool NAK;
int length;
bool las