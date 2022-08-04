#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include<sys/socket.h>
#include <sys/types.h> 
#include <arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include<sqlite3.h>
#include<string.h>

#define   PRINT_ERR(a)   do{fprintf(stderr,"__%d__",__LINE__);perror(a);}while(0)

struct info {
    unsigned int staffno;              //工号
    unsigned int usertype;        // 用户类型
    char name[40];               //姓名
    char  passward[10];     // 密码
    unsigned int  age;          // 年龄
    char  phone_number[15];   //电话
    char   home_address[100];    //地址
    char   postion[40];              //职位
    char   date[15];     //入职时间
    unsigned int  level;      //等级
    double salary;            //薪资
};

//通信结构体
typedef struct {
    int usertype;  //用户类型
    int cmdtype;  // 通信的指令类型
    char buf[32];    //通信的消息
    struct info st;   //员工的信息
}msg_t;


void send_recv_msg(msg_t msg,int sfd);
//主菜单界面
void admin_or_user_login(msg_t msg,int sfd);

//管理用户界面
void admin_menu_ui(msg_t msg,int sfd,char *name);
void do_admin_query(msg_t msg,int sfd);
void do_admin_modification_ui(int sfd, char *name);
void do_admin_adduser_ui(int sfd,char *name);
void do_admin_deluser_ui(int sfd,char *name);
void do_admin_history(int sfd,char *name);

//普通用户界面
void user_menu_ui(msg_t msg,int sfd,char *name);
void do_user_modification_ui(int sfd,char *name);


//查询界面（公用）
void do_search_ui(void);


#endif