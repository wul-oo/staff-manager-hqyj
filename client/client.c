#include"client.h"

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        printf("请输入正确的ip地址与端口号\n");
        return -1;
    }

    int sfd = socket(AF_INET,SOCK_STREAM,0);
    if(sfd < 0)
    {
        PRINT_ERR("socket");
        return -1;
    }

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(argv[1]);
    sin.sin_port = htons(atoi(argv[2]));

    if(connect(sfd,(struct sockaddr *)&sin,sizeof(sin)) < 0 )
    {
        PRINT_ERR("connect");
        return -1;
    }

    printf("connect success\n");
    msg_t msg;
  
    bzero(&msg,sizeof(msg));
    admin_or_user_login(msg,sfd);

     
   
    close(sfd);
    return 0;
}
