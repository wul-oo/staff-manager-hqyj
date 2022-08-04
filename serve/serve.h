#ifndef __SERVE_H__
#define __SERVE_H__

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
#include<time.h>
#define   PRINT_ERR(a)   do{fprintf(stderr,"__%d__",__LINE__);perror(a);}while(0)

struct arguments{
    sqlite3 *db;
    int nsfd;
};

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


//用户登录验证
void log_in(msg_t msg ,sqlite3 *db,int sfd);

//查询用户信息
void search_info(msg_t *msg,sqlite3 *db,int sfd);

//添加用户信息
void adduser_info(msg_t msg ,sqlite3 *db,int sfd);

//修改用户信息
void admin_modify_info(msg_t msg,sqlite3 *db, int sfd);

//查询历史记录
void search_history(sqlite3 *db, int sfd);

void admin_deluser(msg_t msg, sqlite3 *db,int sfd);

#endif