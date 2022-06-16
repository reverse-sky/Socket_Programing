#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<inttypes.h>
#include<arpa/inet.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/un.h>
#include<sys/uio.h>
#include <signal.h>

#define CHAT_SERVER "127.0.0.1"//역시 마찬가지로 서버랑 통신할 부분
#define BUFFER_LENGTH 256

int s,us,usd;
int other; //상대방이 보낸 소켓의 이름을 출력할 변수
char UNIX_PATH[50];
void handler(int sig){
    /*CTRL + C가 입력되면 handler함수가 호출됨 
    여기에 close(socket)구현 
    */
    close(s);//연결용 socket 닫음 
    close(us);
    close(usd);
    printf("(시그널 핸들러 작동) 마무리 작업 시작");
    unlink(UNIX_PATH);

    exit(0);
}

void setNonBlock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);//non block으로 만들기 위해서 세팅 
    // exit_if(flags < 0, "fcntl failed");
    int r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    // exit_if(r < 0, "fcntl failed");
}

int main(int argc, char* argv[]){
    int CHAT_PORT;
    if (argc>1 ||CHAT_PORT<5000 || CHAT_PORT>10000){
        CHAT_PORT = 5000+atoi(argv[1]);
    }
    else if(argc==1){
        CHAT_PORT = 9999;
    }
    else CHAT_PORT = atoi(argv[1]);

    signal(SIGINT,handler);
    struct sockaddr_in client;          //server와 통신할 소켓 
    struct sockaddr_un userver,uclient; //Unix로 client의 입력을 받음
    char ubuf[BUFFER_LENGTH];           //유닉스용 버퍼
    char buf[BUFFER_LENGTH];            //inet통신용 버퍼 
    int uclient_len;                    
    int length = sizeof(buf)+1;
    int check, check_inet;
    int sock_num = 0;
    //  strcpy(UNIX_PATH,"./c1");          //사용자 마다 unix소켓 주소를 따로 받음     
    //     strcat(UNIX_PATH,"");          //소켓 통신할 주소 
   // 입력을 받아서 해도 되지만, debug모드를 사용해서 주소를 할당하도록 변경
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


    memset((char *)&client,'\0',sizeof(client));
    memset((char *)&userver,'\0',sizeof(userver));


    //  unix용 소켓 생성 
    if((us = socket(AF_UNIX,SOCK_STREAM,0))==-1){perror("unix_socket");exit(1);}
    userver.sun_family = AF_UNIX;
    strcpy(userver.sun_path,UNIX_PATH);
    if (bind(us,(struct sockaddr *)&userver  ,sizeof(userver))){perror("unix_bind");exit(1);}
    if(listen(us,5)){perror("unix_listen");exit(1);}
    printf("[Info] Unix socket : waiting for connect req\n");
    //소켓 설정 완료

    if((usd=accept(us,(struct sockaddr *)&uclient, &uclient_len))==-1){perror("accept");exit(1);}
    printf("[Info] Unix socket : client connected\n");
    setNonBlock(usd);//usd를 클라이언트의 이야기를 상시 들을 수 있어야함
    // 동시에 서버로 부터 받는 메시지도 받아야한다.
    

    // inet용 소켓 생성
    if ((s = socket(AF_INET,SOCK_STREAM,0))==-1){perror("socket"); exit(1);}
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(CHAT_SERVER);
    client.sin_port = htons(CHAT_PORT);


    if(connect(s,(struct sockaddr*)&client,sizeof(client))==-1){perror("inet_connect");exit(1);};
    //client소켓 연결 됬으면 밑에 프린트 문 출력됨 
    printf("[Info] Inet socket : connected to the server\n\n\n");
    if((check_inet=recv(s,buf,length,0))==-1){check= 0;}//상대가 보내는거 기다림
    sock_num = atoi(buf);
    printf("my_socket_num: %d\n",sock_num);

    setNonBlock(s);//non block으로 설정 


    while(1){
        if((check_inet=recv(s,buf,length,0))==-1){check= 0;}//상대가 보내는거 기다림                 
        if(check_inet>0){//서버가 보낸게 있으면 
            if(strcmp(buf,"\\exit\n")==0)break;
            char *ptr = strtok(buf,"*");
            while(ptr !=NULL){
                ptr = strtok(NULL,"*");
                if(ptr!=NULL)other = atoi(ptr);// *뒤에는 소켓 번호가 있다고 가정하고, 
            }//소켓 번호와 메세지를 함께 출력
            if(other==sock_num)printf("[ME] : %s\n",buf);
            else if(other>0)printf("[%d] : %s\n",other,buf);
            else printf("%s",buf);
        }
        if((check=recv(usd,ubuf,length,0))==-1){continue;}//평상시의 상태, 내가 보내는거 기다림 
        if(check>0){//읽은게 있을 때만 출력함
            // printf("[ ME ] : %s\n",ubuf);
            // if (strcmp(ubuf,"\\quit\n")==0){break;}
            // if(strcmp(buf,"0\n")==0||strcmp(buf,"1\n")==0||strcmp(buf,"2\n")==0)
            // printf("[ME] : %s\n",buf);

            }
            if(send(s,ubuf,length,0)==-1){perror("send");exit(0);}//내가 받은거 상대한테 보냄



    }



    // while(1){
    //     check_inet=recv(s,buf,length,0);//상대가 보내는거 기다림                 
    //     if(check_inet>0) printf("%s",buf);
    //     fgets(buf,BUFFER_LENGTH,stdin);
    //     if(send(s,buf,length,0)==-1){continue;}


    //     if (strcmp(buf,"\\quit\n")==0){
    //         printf("program_exit");
    //         break;
    //     }    
    //     if(recv(s,buf,length,0)==-1){continue;}
    //     printf("server said: %s\n",buf);
   
    // }
    printf("[Info] Closing socket");
    close(s);//연결용 socket 닫음 
    close(us);
    close(usd);
    return 0;
}