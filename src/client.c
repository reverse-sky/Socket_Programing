#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/un.h>
#include<sys/uio.h>
#define BUFFER_LENGTH 256
int s,sd;
char UNIX_PATH[50];


void handler(int sig){
    /*CTRL + C가 입력되면 handler함수가 호출됨 
    여기에 close(socket)구현 
    */
    close(s);//연결용 socket 닫음 
    close(sd);//accept 소켓 닫음 
    unlink(UNIX_PATH);
    printf("client 종료");

}
int main(){
    struct sockaddr_un client;
    char buf[BUFFER_LENGTH];
    int length = BUFFER_LENGTH;
    memset((char *)&client,'\0',sizeof(client));

    #ifdef c1
        strcpy(UNIX_PATH,"./c1");    //사용자 마다 unix소켓 주소를 따로 받음     
    #endif
    #ifdef c2
        strcpy(UNIX_PATH,"./c2");    //사용자 마다 unix소켓 주소를 따로 받음
    #endif
    #ifdef c3
        strcpy(UNIX_PATH,"./c3");    //사용자 마다 unix소켓 주소를 따로 받음     
    #endif
    #ifdef c4
        strcpy(UNIX_PATH,"./c4");    //사용자 마다 unix소켓 주소를 따로 받음     
    #endif


    if((s = socket(AF_UNIX,SOCK_STREAM,0))==-1){perror("unix_socket");exit(1);}
    client.sun_family = AF_UNIX;
    strcpy(client.sun_path,UNIX_PATH);

    if(connect(s,(struct sockaddr*)&client,sizeof(client))==-1){perror("unix_connect");exit(1);}

    while(1){
        printf("Enter:  ");
        fgets(buf,BUFFER_LENGTH,stdin);
        if(send(s,buf,length,0)==-1){perror("send");exit(1);}
        // if (strcmp(buf,"\\quit\n")==0){printf("Terminate.....");break;}    
        //fgets는 \n까지 읽기 때문에 마지막에 \n을 붙여야함 
    }
    printf("Success\n");
    printf("[Info] Closing socket");
    close(s);
    close(sd);

}