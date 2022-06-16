#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/select.h>

#include<inttypes.h>
#include<sys/stat.h>
#include<fcntl.h>   //blocking을 제어하기 위한 통신 
#include <sys/un.h> //unix 통신
#include<sys/uio.h> //unix 통신 
#include <signal.h> //sigint를 처리하기 위해서 부르는 .h파일 
#include<pthread.h> //thread를 관리하기 위해서 선언하는 .h파일 


#define BUFFER_LENGTH 256
#define SOCKET_NUM 15
#define CHATROOM 3
//선언부
struct ChatRoom{
    int room_num;   //채팅방 번호
    int user_num;   //채팅방 사용자 수 max =5
    // int chat_member[15];
};

struct UserSockets{
    int sock_num;
    int index;
};
typedef struct ChatInfo{
    struct ChatRoom room;           //채팅방에 관한 정보 저장을 위한 구조체
    struct UserSockets new_users;      //채팅방에 새로 참여하는 사용자들 
    struct UserSockets returned_users; //채팅방에서 탈퇴한 사용자들 
}ChatInfo;

int s,sd[SOCKET_NUM]={0,};
int array[SOCKET_NUM]={0,};  //어느 채팅방에 있는지를 표시하는 배열  1 ,2,3가 들어있음
void handler(int sig){
    /*CTRL + C가 입력되면 handler함수가 호출됨 
    여기에 close(socket)구현 
    */
    printf("(시그널 핸들러 작동)마무리 작업 시작");
    close(s);//연결용 socket 닫음 
    for(int i=0;i<SOCKET_NUM;i++){
        close(sd[i]);
    }
    exit(0);

}
void setNonBlock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    // exit_if(flags < 0, "fcntl failed");
    int r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    // exit_if(r < 0, "fcntl failed");
}
int maxArr(int *s,int N_CLIENT){
    int max = 0;
    for(int i=0;i<N_CLIENT;i++){
        if(max<s[i])max = s[i];
    }
    return max;
}
void *start_thread(void *arg){
    ChatInfo *chatinfo = (ChatInfo *)arg;
    char buf[BUFFER_LENGTH];
    int length = BUFFER_LENGTH+1;
    int check_thread, check_user;
    int N_CLIENT = 0;
    int ret =0;
    fd_set readfds;     //fd_set 초기화     
    int index = 0;
    char dumy[20];
    struct timeval tv;
    tv.tv_sec =3;
    tv.tv_usec = 500;
    while(1){
        // printf("[Ch.%d] 새로운 참가자 : %d\n",chatinfo->room.room_num,sd[index]);

        if (chatinfo->new_users.sock_num>0){
            index = chatinfo->new_users.index;
            chatinfo->room.user_num++;
            chatinfo->new_users.sock_num=-1;
            chatinfo->new_users.index=-1;
            N_CLIENT = chatinfo->room.user_num;

            // array[index] = chatinfo->room.room_num;
            // chatinfo->room.chat_member[index]=chatinfo->room.room_num;//room으로 이동시킴
            printf("[Ch.%d] 새로운 참가자 : %d\n",chatinfo->room.room_num,sd[index]);
        }
        // if(N_CLIENT ==0){
            // sleep(3);
            // printf("%d  hi\n",chatinfo->room.room_num);
            // }

        if(N_CLIENT>1){

        
            FD_ZERO(&readfds);//읽을 FD_SET을 일단 0을 초기화 
            int j=0;
            int check[15];
            for(int i=0;i<SOCKET_NUM;i++){
                if(array[i] ==chatinfo->room.room_num){
                    FD_SET(sd[i],&readfds);//소켓 기술자를 set집합에 추가 매번 해야함
                    check[j]=i;
                    j++;
                }
                    //room에 해당하는 소켓만 select에 넣음 
            }
            
            ret = select(maxArr(sd,15)+1,&readfds,NULL,NULL,&tv);
            // printf("select returned: %d\n",ret);
            // printf("%d",ret);
            switch(ret){
                case -1:
                    perror("error");
                case 0:
                    continue;
                    // printf("select returns: 0\n");
                default: 
                    int i=0;//index for s array
                    while (ret>0)
                    {
                        if(FD_ISSET(sd[check[i]],&readfds)){//sd[i]에 해당하는 게 읽어올 게 있으면 0이 아닌 수 return
                            memset(buf,0,length);
                            if(recv(sd[check[i]],buf,length,0)==-1){perror("recv");break;}
                            printf("[Ch.%d]사용자 %d가 보낸 매세지 : %s\n",chatinfo->room.room_num,array[i],buf);
                            // printf("%d\n",sd[check[i]]);
                            ret --;
                            if(strcmp(buf,"\\quit\n")==0){
                                array[check[i]] = 0;
                                chatinfo->room.user_num--;
                                chatinfo->returned_users.sock_num = sd[check[i]];
                                chatinfo->returned_users.index = check[i];
                                // memset(buf,0,length);
                                continue;
                            }
                            for(int k=0;k<N_CLIENT;k++){
                                sprintf(dumy,"*%d",sd[check[i]]);
                                strcat(buf,dumy);
                                if(send(sd[check[k]],buf,length,0)==-1){perror("send");exit(0);}//내가 받은거 상대한테 보냄
                                if(N_CLIENT==1)printf("[CH.%d]사용자 %d가 혼자여서 메시지를 보내지 않습니다.\n",chatinfo->room.room_num,sd[index]);
                                
                                // printf("[Ch.%d]사용자 %d가 보낸 매세지 : %s",chatinfo->room.room_num,array[i],buf);
                            }

                        }else;
                        i++;
                    }
                    break;
            }   
        }
    }

    // pthread_exit(0);
};



//signal 보냈을 떄 처리 


int main(int argc, char* argv[]){
    int CHAT_PORT;
    if (argc>1 ||CHAT_PORT<5000 || CHAT_PORT>10000){
        CHAT_PORT = 5000+atoi(argv[1]);
    }
    else if(argc==1){
        CHAT_PORT = 9999;
    }
    else CHAT_PORT = atoi(argv[1]);
    printf("server port : %d\n",CHAT_PORT);
    signal(SIGINT,handler);
    struct sockaddr_in server,client;
    char buf[BUFFER_LENGTH];//inet통신용 버퍼 
    int length = BUFFER_LENGTH+1;
    int N_CLIENT = sizeof(sd)/sizeof(int);
    int client_len;
    int client_num=0;
    int ret=0;
    int current_socket = 0;
    fd_set readfds;     //fd_set 초기화     
    char *MENU= "<MENU>\n1.채팅방 목록 보기\n2.채팅방 참여하기(사용법: 2 <채팅방 번호>))\
    \n3.프로그램 종료\n (0을 입력하면 메뉴가 다시표기됩니다.)\n\n";
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500;    
    
    ChatInfo chatinfo[CHATROOM];
    pthread_t tID[CHATROOM];
    //thread생성 
    for(int i=0;i<CHATROOM;i++){
        chatinfo[i].room.room_num=i+1;
        chatinfo[i].new_users.index =-1;
        chatinfo[i].room.user_num=0;
        chatinfo[i].new_users.sock_num=-1;
        chatinfo[i].returned_users.sock_num=-1;
        // memset(chatinfo[i].room.chat_member,0,sizeof(chatinfo[i].room.chat_member));
        pthread_create(&tID[i],NULL,start_thread,(void*)&chatinfo[i]);
    
    }
    // inet용 소켓 생성
    if ((s = socket(AF_INET,SOCK_STREAM,0))==-1){perror("socket"); exit(1);}

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(CHAT_PORT);

    //생성된 소켓에 주소를 부여, 연결 매핑함
    if (bind(s,(struct sockaddr *)&server  ,sizeof(server))){perror("bind");exit(1);}
    if(listen(s,15)){perror("listen");exit(1);}
    printf("[Info] Inet socket : waiting for connect req\n");
    
    // pthread_join(tID,NULL);//join 필요 없음
    for(int i=0;i<4;i++){//client가 accept될 때 까지 기다림
        if((sd[i]=accept(s,(struct sockaddr *)&client, &client_len))==-1){perror("accept");exit(1);}
        else printf("[MAIN] 새로운 사용자가 접속했습니다 : %d \n",sd[i]);//4개의 socket은 한번에 입력받음
        setNonBlock(sd[i]);//select하기 위해서 nonblock으로 설정 
        sprintf(buf,"%d",sd[i]);//숫자를 문자열로 변환
        if(send(sd[i],buf,length,0)==-1){perror("send");exit(0);}//자기 자신의 소켓 번호를 보냄
        sleep(0.5);
        if(send(sd[i],MENU,length,0)==-1){perror("send");exit(0);}//내가 받은거 상대한테 보냄
        current_socket++;
    }

    // for(int i=0;i<N_CLIENT;i++){

    // }
    while(1){
        // if(current_socket<0){
        for(int i=0;i<3;i++){
            // printf("%d\n",chatinfo[i].returned_users.sock_num);
            if(chatinfo[i].returned_users.sock_num>0){
                int idx = chatinfo[i].returned_users.index;
                chatinfo[i].returned_users.sock_num=-1;
                printf("[MAIN] 채팅방 탈퇴 사용자 탐지 :%d\n",sd[idx]);
                if(send(sd[idx],MENU,length,0)==-1){perror("send");exit(0);}//내가 받은거 상대한테 보냄
                current_socket++;
            }
        }
        // }
        if(current_socket>0){
            FD_ZERO(&readfds);//읽을 FD_SET을 일단 0을 초기화 
            for(int i=0;i<15;i++){
                // printf("array%d ",array[i]);
                if (array[i]==0){
                    FD_SET(sd[i],&readfds);
                }//소켓 기술자를 set집합에 추가 매번 해야함
            }
            int delete=-1;
            ret = select(maxArr(sd,N_CLIENT)+1,&readfds,NULL,NULL,&tv);
            //time val안해주면 인식 못함 이거때문에 시간 오지게 걸림 
            switch(ret){
                case -1:
                    perror("error");
                case 0:
                    continue;
                    // printf("select returns: 0\n");break;
                default: 
                    int i=0;//index for s array
                    while (ret>0)
                    {
                        if(FD_ISSET(sd[i],&readfds)){//sd[i]에 해당하는 게 읽어올 게 있으면 0이 아닌 수 return
                            memset(buf,0,length);
                            if(recv(sd[i],buf,length,0)==-1){perror("recv");break;}
                            ret --;
                            printf("[MAIN]사용자 %d 메세지 : %s\n",sd[i],buf);
                            // printf("%d\n",strcmp(buf,"1\n"));
                            char *ptr = strtok(buf," ");//띄어쓰기는 일단 끊음

                            if(strcmp(buf,"0\n")==0){//메뉴판 다시 보냄 
                                if(send(sd[i],MENU,length,0)==-1){perror("send");exit(0);}
                            }
                            if(strcmp(buf,"1\n")==0){
                                memset(buf,0,length);//buf새로 세팅
                                char dumy[30];
                                strcpy(buf,"<ChatRoom info>\n");
                                for(int j=0;j<3;j++){
                                    sprintf(dumy,"[ID: %d] Chatroom - %d (%d/%d)\n",chatinfo[j].room.room_num,\
                                    chatinfo[j].room.room_num,chatinfo[j].room.user_num,5);
                                    strcat(buf,dumy);
                                }
                                strcat(buf,"\n");
                                if(send(sd[i],buf,length,0)==-1){perror("send");exit(0);}

                            }
                            if(strcmp(buf,"2")==0){
                                // if(send(sd[i],buf,length,0)==-1){perror("send");exit(0);}
                                // char *ptr = strtok(buf," ");//접속하고, 뒤에 부분은 원하는 방 번호
                                // printf("current: 2\n");
                                int num =0;
                                while(ptr !=NULL){
                                    // printf("num: %s",ptr);
                                    ptr = strtok(NULL," ");
                                
                                    if(ptr!=NULL)num = atoi(ptr);// *뒤에는 소켓 번호가 있다고 가정하고, 
                                }
                                if(num==1 ||num ==2||num ==3){ 
                                printf("[MAIN] 사용자 %d가 채팅방 %d 에 참여합니다.\n",sd[i],num);
                                chatinfo[num-1].new_users.sock_num = sd[i];
                                chatinfo[num-1].new_users.index = i;
                                array[i] = num;
                                current_socket--;
                                }
                            }
                            if(strcmp(buf,"3\n")==0){
                                printf("[MAIN] %d 사용자와의 접속을 해제합니다.\n",sd[i]);
                                memset(buf,0,length);
                                strcpy(buf,"\\exit\n");
                                if(send(sd[i],buf,length,0)==-1){perror("send");exit(0);}//내가 받은거 상대한테 보냄

                                // if(cur)
                                array[i] = 99;
                                current_socket--;

                            }

                        }else;
                        i++;
                    }
                    break;
            }
            // close(delete);
            // delete = -1;
        }
    }


//2: 채팅 방에서 탈퇴한 사용자가 있는지 확인 


//3: 클라이언트의 요청 처리
    // while(1){
    //     FD_ZERO(&readfds);//읽을 FD_SET을 일단 0을 초기화 

    //     for(int i=0;i<N_CLIENT;i++){FD_SET(sd[i],&readfds);}//소켓 기술자를 set집합에 추가 매번 해야함
    //     printf("waiting at select..\n");
    //     ret = select(maxArr(sd,N_CLIENT)+1,&readfds,NULL,NULL,&tv);
    //     printf("select returned: %d\n",ret);
    //     switch(ret){
    //         case -1:
    //             perror("error");
    //         case 0:
    //             printf("select returns: 0\n");break;
    //         default: 
    //             int i=0;//index for s array
    //             while (ret>0)
    //             {
    //                 if(FD_ISSET(sd[i],&readfds)){//sd[i]에 해당하는 게 읽어올 게 있으면 0이 아닌 수 return
    //                     memset(buf,0,length);
    //                     if(recv(sd[i],buf,length,0)==-1){perror("recv");break;}
    //                     ret --;
    //                     if((strcmp(buf,"\\quit")==0))close(sd[i]);
    //                     printf("MSG from client %d: %s\n\n\n",i,buf);
    //                 }else;
    //                 i++;
    //             }
    //             break;
    //     }

    // }
    
    // whlie(1){
    //     if (3){

    //     }
    // }
    return 0;
}


