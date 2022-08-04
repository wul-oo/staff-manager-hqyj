#include"serve.h"

#define MAX 50


void *connect_fun(void * arg);

int main(int argc,const char * argv[])
{
   if(argc != 3)
   {
    printf("请输入正确的ip地址与端口号\n");
    return -1;
   }
    struct arguments arg;
    //打开库
    sqlite3 * db;
    if(sqlite3_open("./staff_manage_system.db",&db) != 0 )
    {
        sqlite3_errmsg(db);
        return -1;
    }
    arg.db = db;
    printf("the database open success.\n");

    //创建表
    char sql[400] = "";
    sprintf(sql,"%s%s ","create table if not exists usrinfo (staffno int primary key, usertype int , name char ,",
    "passwd char, age int , phone char , addr char, postion char, date char, level int , salary double )");
  
    char *errmsg;
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
    {
        printf("%s\n", errmsg);
        return -1;
    }
    printf("table usrinfo already exists.\n");

    bzero(sql, sizeof(sql));
    strcpy(sql, "create table if not exists historyinfo ( time char , name char , words char)");
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
    {
        printf("%s\n", errmsg);
        return -1;
    }
    printf("table hsitroyinfo already exists.\n");

    //创建服务器
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0)
    {
        PRINT_ERR("socket");
        return -2;
   }

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(atoi(argv[2]));
    sin.sin_addr.s_addr = inet_addr(argv[1]);
   
    if(bind(sfd,(struct sockaddr *)&sin,sizeof(sin)) < 0 )
    {
        PRINT_ERR("bind");
        return -3;
    }

    if(listen(sfd,MAX) < 0 )
    {
        PRINT_ERR("listen");
        return -4;
    }


    struct sockaddr_in cin;
    socklen_t addrlen;
    int nsfd;
    int ret;
    pthread_t tid;
    while (1)
    {
        nsfd = accept(sfd, (struct sockaddr *)&cin, &addrlen);
        if (nsfd < 0)
        {
            PRINT_ERR("accept");
            break;
        }
        printf("nsfd = %d connect success\n",nsfd);

        //有一个客户端登录就产生一个线程进行处理
        arg.nsfd = nsfd;
        ret = pthread_create(&tid,NULL,connect_fun,&arg);
        if(ret < 0 )
        {
            PRINT_ERR("pthread_create");
            break;
        }
       

    }
    close(sfd);
    return 0;
}


void *connect_fun(void *arg)
{
    pthread_detach(pthread_self());
    sqlite3 *db =  ((struct arguments *)arg)->db;
    int nsfd = ((struct arguments *)arg)->nsfd;
    msg_t msg;
    memset(&msg,0,sizeof(msg));
    int ret;
    while(1)
    {
        //接收消息
        bzero(&msg,sizeof(msg));
        ret = recv(nsfd,&msg,sizeof(msg),0);
        if(ret == 0)
        {
            printf("sfd = %d 退出登录\n",nsfd);
            break;
        }
        else if(ret < 0)
        {
            PRINT_ERR("recv");
            break;
        }
        //判断是哪种类型的信息
        printf("msg.cmdtype = %d\n",msg.cmdtype);
        switch(msg.cmdtype)
        {
            case 0 :   //登录  
            printf("%s:%d:%d\n",__func__,__LINE__,msg.usertype);
            log_in(msg,db,nsfd);
            break;
            printf("%s:%d:%d\n",__func__,__LINE__,msg.usertype);
            case 1: //查询
            search_info(&msg,db,nsfd);
            break;
             case 2: //修改
             printf("%s:%d:%d,%d\n",__func__,__LINE__,msg.usertype,msg.st.usertype);
                admin_modify_info(msg,db,nsfd);
            break;

             case 3:   //添加
            adduser_info(msg ,db,nsfd);
            break;    

             case 4:   //删除
             admin_deluser(msg,db,nsfd);
            break;    

             case 5:   //修改的历史记录
            search_history(db,nsfd);
            break;
        }

    }


    close(nsfd);
    pthread_exit(NULL);
}

