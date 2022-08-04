#include"serve.h"

//用户登录验证
void log_in(msg_t msg ,sqlite3 *db,int sfd)
{
    char sql[300];
   
   if(msg.usertype == 1)
   {
    sprintf(sql, "%s\"%s\" and usertype=1 ;","select passwd from usrinfo where name=",
            msg.st.name);
   }
   else
   {
     sprintf(sql, "%s\"%s\" and usertype=0 ;","select passwd from usrinfo where name=",
            msg.st.name);
   }
    
   printf("%d:%s\n",__LINE__,sql);
    char **result;
    int row;
    int column;
    char *errmsg;

    if(sqlite3_get_table(db,sql,&result,&row,&column,&errmsg) != 0 )
    {
        printf("__%d__:%s\n",__LINE__,errmsg);
        return ;  //查询出错
    }

    if(row == 0)
    {
        //printf("no user\n");
        strcpy(msg.buf,"用户不存在，请先注册");
    }
    else
    {
       
        //判断密码是否正确
        if (!strcmp(result[row], msg.st.passward))
        {
           // printf("user: %s login success\n",msg.st.name);
             bzero(msg.buf, sizeof(msg.buf));
            strcpy(msg.buf, "success");
        }
        else
        {
             bzero(msg.buf, sizeof(msg.buf));
            strcpy(msg.buf, "密码错误");
        }

    }

    if (send(sfd, &msg, sizeof(msg), 0) < 0)
    {
        printf("%s:%d:send failed\n", __func__, __LINE__);
    }

    sqlite3_free_table(result);
}

//查询用户信息
void search_info(msg_t *msg,sqlite3 *db,int sfd)
{
    char sql[128] = "";
    switch (msg->usertype)
    {
    case 0:   //管理员模式
        if (!strcmp(msg->buf, "all")) //查询所有
        {
            bzero(sql,sizeof(sql));
            strcpy(sql,"select * from usrinfo ");
        }
        else
        {
            //根据姓名查询
            bzero(sql,sizeof(sql));
            sprintf(sql,"%s\"%s\" ","select * from usrinfo where name=",msg->st.name);
        }
        break;
    
    case 1:   //普通用户模式
            //根据姓名查询---只能查看自己的
            bzero(sql,sizeof(sql));
            sprintf(sql,"%s\"%s\" ","select * from usrinfo where name=",msg->st.name);
          //  printf("%d:%s\n",__LINE__,sql);
        break;
    }

    char **result;
    int row;
    int column;
    char *errmsg;

    if(sqlite3_get_table(db,sql,&result,&row,&column,&errmsg) != 0 )
    {
        printf("%s\n",errmsg);
        return ;  //查询出错
    }
   
    int i = 0 ;
    msg_t s_msg;
    printf("column = %d,row = %d\n", column, row);
    for (i = column; i < (row + 1) * column; i = i + column)
    {
        bzero(&s_msg, sizeof(s_msg));
        s_msg.cmdtype = 0;
        s_msg.st.staffno = atoi(result[i]);
        s_msg.st.usertype = atoi(result[i + 1]);
        strcpy(s_msg.st.name, result[i + 2]);
        strcpy(s_msg.st.passward, result[i + 3]);
        s_msg.st.age = atoi(result[i + 4]);
        strcpy(s_msg.st.phone_number,result[i + 5]);
        strcpy(s_msg.st.home_address, result[i + 6]);
        strcpy(s_msg.st.postion, result[i + 7]);
        strcpy(s_msg.st.date, result[i + 8]);
        s_msg.st.level = atoi(result[i + 9]);
        s_msg.st.salary = atof(result[i + 10]);
        if (i == column * row  )
        {
            strcpy(s_msg.buf, "send complete"); //让客户端知道数据发送完成
        }
        if (send(sfd, &s_msg, sizeof(s_msg), 0) < 0)
        {
            printf("send failed\n");
            break;
        }
    }
    if(row == 0)
    {
        bzero(&s_msg,sizeof(s_msg));
        strcpy(s_msg.buf, "no userinfo");
        if (send(sfd, &s_msg, sizeof(s_msg), 0) < 0)
        {
            printf("send failed\n");
        }
    }
    sqlite3_free_table(result);
}

//添加用户信息的同时，添加历史记录表
void adduser_info(msg_t msg ,sqlite3 *db,int sfd)
{

    //保存一下管理员的名字
    char manager_name[40] = "";
    printf("manager_name = %s\n",msg.buf);
    strcpy(manager_name,msg.buf);
    char sql[300];

    printf("msg.uertype = %d\n",msg.usertype);
    if (msg.usertype == 0)
    {
        sprintf(sql, "%s(%d, 0 ,'%s','%s',%d,'%s','%s','%s','%s',%d,%lf);", "insert into usrinfo values ",
                msg.st.staffno, msg.st.name, msg.st.passward, msg.st.age,
                msg.st.phone_number, msg.st.home_address, msg.st.date, msg.st.postion, msg.st.level, msg.st.salary);
    }
    else
    {
        sprintf(sql, "%s(%d, 1 ,'%s','%s',%d,'%s','%s','%s','%s',%d,%lf);", "insert into usrinfo values ",
                msg.st.staffno, msg.st.name, msg.st.passward, msg.st.age,
                msg.st.phone_number, msg.st.home_address, msg.st.date, msg.st.postion, msg.st.level, msg.st.salary);
    }

    printf("__%d__:%s\n",__LINE__,sql);

    char *errmsg = NULL;
    if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) < 0)
    {
        printf("%s--%s--%d:%s\n",__FILE__,__func__,__LINE__,errmsg);
        return ;
    }
     printf("insert userinfo success\n");
    //插入历史记录表
    time_t t = time(NULL);
    struct tm * tm = localtime(&t);
    bzero(sql,sizeof(sql));
    sprintf(sql,"insert into historyinfo values ('%d.%d.%d %4d:%02d:%02d','%s','管理员%s添加了%s用户')",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,
    tm->tm_hour,tm->tm_min,tm->tm_sec,manager_name,manager_name,msg.st.name);

    if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) < 0)
    {
        printf("%s--%s--%d:%s\n",__FILE__,__func__,__LINE__,errmsg);
        return ;
    }


    printf("insert historyinfo success\n");

    bzero(msg.buf,sizeof(msg.buf));
    strcpy(msg.buf,"adduser success");
    if(send(sfd,&msg,sizeof(msg),0) < 0)
    {
        PRINT_ERR("send");
        return ;
    }


}

//查询历史记录
void search_history(sqlite3 *db, int sfd)
{
    char sql[128] = "select * from historyinfo ";

    char **result;
    int row;
    int column;
    char *errmsg;

    if (sqlite3_get_table(db, sql, &result, &row, &column, &errmsg) != 0)
    {
        printf("%s\n", errmsg);
        return; //查询出错
    }

    int i = 0;
    msg_t s_msg;
    printf("column = %d,row = %d\n", column, row);
    for (i = column; i < (row + 1) * column; i = i + column)
    {
        bzero(&s_msg, sizeof(s_msg));
        s_msg.cmdtype = 5;
        //利用date传time
        strcpy(s_msg.st.date,result[i]);
      
        //利用name 传name;
        strcpy(s_msg.st.name,result[i+1]);
      
        //利用address 传words
        strcpy(s_msg.st.home_address,result[i+2]);
       

        if (i == column * row)
        {
            strcpy(s_msg.buf, "send complete"); //如何让客户端知道数据发送完成？
        }
        if (send(sfd, &s_msg, sizeof(s_msg), 0) < 0)
        {
            printf("send failed\n");
            break;
        }
    }
    if (row == 0)
    {
        bzero(&s_msg, sizeof(s_msg));
        strcpy(s_msg.buf, "no userinfo");
        if (send(sfd, &s_msg, sizeof(s_msg), 0) < 0)
        {
            printf("send failed\n");
        }
    }
    sqlite3_free_table(result);
}

//修改用户信息
void admin_modify_info(msg_t msg,sqlite3 *db, int sfd)
{
    char sql[200] = "";
    char manager_name[40] = "";
    strcpy(manager_name, msg.buf + 2); //获取操作者的名字
    char type[15] = "";                //获取更改类型

    printf("************%s************\n", manager_name);
    switch (*msg.buf) //根据buf的第一位判断修改类型
    {
    case 1:
        sprintf(sql, "update usrinfo set name='%s' where staffno=%d ", msg.st.name, msg.st.staffno);
        strcpy(type, "姓名");
        break;
    case 2:
        sprintf(sql, "update usrinfo set age=%d where staffno=%d ", msg.st.age, msg.st.staffno);
        strcpy(type, "年龄");
        break;
    case 3:
        sprintf(sql, "update usrinfo set addr='%s' where staffno=%d ", msg.st.home_address, msg.st.staffno);
        strcpy(type, "住址");
        break;
    case 4:
        sprintf(sql, "update usrinfo set phone='%s' where staffno=%d ", msg.st.phone_number, msg.st.staffno);
        strcpy(type, "电话");
        break;
    case 5:
        sprintf(sql, "update usrinfo set postion='%s' where staffno=%d ", msg.st.postion, msg.st.staffno);
        strcpy(type, "职位");
        break;
    case 6:
        sprintf(sql, "update usrinfo set salary=%lf where staffno=%d ", msg.st.salary, msg.st.staffno);
        strcpy(type, "薪资");
        break;
    case 7:
        sprintf(sql, "update usrinfo set date='%s' where staffno=%d ", msg.st.date, msg.st.staffno);
        strcpy(type, "入职时间");
        break;
    case 8:
        sprintf(sql, "update usrinfo set level=%d where staffno=%d ", msg.st.level, msg.st.staffno);
        strcpy(type, "评级");
        break;
    case 9:
        sprintf(sql, "update usrinfo set passwd='%s' where staffno=%d ", msg.st.passward, msg.st.staffno);
        strcpy(type, "密码");
        break;
    }
    printf("sql = %s\n",sql);
    char *errmsg;
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
    {
        printf("%s:%d:%s\n", __func__, __LINE__, errmsg);
        bzero(msg.buf, sizeof(msg.buf));
        strcpy(msg.buf, "modify failed");
        send(sfd, &msg, sizeof(msg), 0);
        printf("修改信息失败\n");
        return;
    }
        //发送修改成功给客户端
    bzero(msg.buf,sizeof(msg.buf));
    strcpy(msg.buf,"modify success");
    send(sfd,&msg,sizeof(msg),0);        
    printf("修改信息成功\n");

        //插入历史记录表
    char pre[20] ="";
    printf("323: usertype = %d, st.usertype = %d\n",msg.usertype,msg.st.usertype);
    if(msg.st.usertype == 0)
    {
        strcpy(pre,"管理员");
    }
    else
    {
        strcpy(pre,"用户");
    }

    time_t t = time(NULL);
    struct tm * tm = localtime(&t);
    bzero(sql,sizeof(sql));
    sprintf(sql,"insert into historyinfo values ('%d.%d.%d %4d:%02d:%02d','%s','%s%s修改了工号为%d的%s')",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,
    tm->tm_hour,tm->tm_min,tm->tm_sec,manager_name,pre,manager_name,msg.st.staffno,type);

    if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
    {
        printf("%s--%s--%d:%s\n",__FILE__,__func__,__LINE__,errmsg);
        return ;
    }


}

//删除用户信息
void admin_deluser(msg_t msg, sqlite3 *db,int sfd)
{
    char manager_name[40] = "";
    strcpy(manager_name,msg.buf);

    char sql[128] = "";
    sprintf(sql,"delete from usrinfo where staffno=%d and name='%s'",msg.st.staffno,msg.st.name);
    printf("sql = %s\n",sql);

    char *errmsg = NULL;

    if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) !=0)
    {
        printf("%s:%s:%d:%s\n",__FILE__,__func__,__LINE__,errmsg);
        bzero(msg.buf,sizeof(msg.buf));
        strcpy(msg.buf,"delete failed");
        send(sfd,&msg,sizeof(msg),0);
        printf("删除失败\n");
        return;
    }

    //发送删除成功的消息
    bzero(msg.buf, sizeof(msg.buf));
    strcpy(msg.buf, "delete success");
    send(sfd, &msg, sizeof(msg), 0);

    //保存删除历史记录

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    bzero(sql, sizeof(sql));
    sprintf(sql, "insert into historyinfo values ('%d.%d.%d %4d:%02d:%02d','%s','%s删除了工号%d')", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec, manager_name, manager_name, msg.st.staffno);

    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
    {
        printf("%s--%s--%d:%s\n", __FILE__, __func__, __LINE__, errmsg);
        return;
    }
}