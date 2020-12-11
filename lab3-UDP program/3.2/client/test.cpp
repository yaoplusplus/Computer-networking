#include<iostream>
using namespace std;

int main()
{
    int len = 420;
    char *p=new char[2];
    p[1]='a';
    p[2]=len;
int r[8];

r[7] = p[2] & 0x01; // 最低道位专属
r[6] = ( p[2] & 0x02) >> 1;
r[5] = ( p[2] & 0x04) >> 2;
r[4] = ( p[2] & 0x08) >> 3;
r[3] = ( p[2] & 0x10) >> 4;
r[2] = ( p[2] & 0x20) >> 5;
r[1] = ( p[2] & 0x40) >> 6;
r[0] = ( p[2] & 0x80) >> 7; // 最高位

for(int i = 0; i < 8; ++i){
    printf(" %d ",r[i]);
} 
    int cout;
    cout<<p[2]<<endl;
    for(int i=0;i<p[2];i++)
    {
        
    }
    printf("over\n");

    if(p[2]<51)
    cout<<"可以比较\n";
    
}