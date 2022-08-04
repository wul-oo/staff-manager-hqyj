#include"client.h"

void admin_or_user_login(msg_t msg,int sfd)
{
    int choice;
    while (1)
    {
        bzero(&msg,sizeof(msg));
        msg.cmdtype = 0;    //登录命令
        printf("*************************************************************\n");
        printf("*********  1:管理员模式    2:普通用户模式    3:退出**********\n");
        printf("*************************************************************\n");
        printf("请输入你的选择>>");
        scanf("%d",&choice);
        while (getchar() != 10);
        switch (choice)
        {

        case 1:
            msg.usertype = msg.st.usertype = 0;  
            break;
        case 2: 
            msg.usertype = msg.st.usertype = 1;
            break;
        case 3:
            return ;
        default:
            printf("输入有误请重新输入,输入任意字符清屏\n");
            while (getchar() != 10);
        }
        if(choice != 1 && choice != 2 && choice != 3)
        {
             system("clear");
             continue;
        }

        printf("*********************admin_or_user_login*********************\n");
        printf("请输入用户名:");
        if(fgets(msg.st.name,sizeof(msg.st.name),stdin) == NULL)
        {
            PRINT_ERR("fgets");
        }
        msg.st.name[strlen(msg.st.name)-1] = 0;
        
        printf("请输入密码(小于10位)>>");
        scanf("%s",msg.st.passward);
        while (getchar() != 10);

        //printf("%s:%d:%d\n",__func__,__LINE__,msg.usertype);

        if(send(sfd,&msg,sizeof(msg),0) < 0)
        {
            PRINT_ERR("send");
            return ;
        }

        bzero(&msg,sizeof(msg));
        if (recv(sfd, &msg, sizeof(msg), 0) < 0)
        {
            PRINT_ERR("send");
            return;
        }

        if (!strcmp(msg.buf, "success"))
        {
            //密码正确
            if(msg.usertype == 0)
            {
                //printf("%s:%d:msg.st.name = %s\n",__func__,__LINE__,msg.st.name);
                admin_menu_ui(msg, sfd,msg.st.name);
                
            }
            else
            {
                //printf("%s:%d:msg.st.name = %s,msg.usertype = %d,msg.st.usertype = %d\n",__func__,__LINE__,msg.st.name,msg.usertype,msg.st.usertype);
                user_menu_ui(msg,sfd,msg.st.name);
            }
            
        }
        else
        {
             printf("%s\n", msg.buf);
        }
       
        printf("输入任意字符清屏\n");
        while (getchar() != 10)
            ;
        system("clear");
    }
}
//管理员模式下的功能
void admin_menu_ui(msg_t msg,int sfd,char *name)
{   
    int choice;
    while (1)
    {
        printf("********************admin_menu*******************************\n");
        printf("*    1:查询  2:修改 3:添加用户  4:删除用户  5:查询历史记录  *\n");
        printf("*    6:退出													*\n");
        printf("*************************************************************\n");
        printf("请输入您的选择(数字)>>");
        scanf("%d", &choice);
        while (getchar() != 10)
            ;
        switch (choice)
        {
        case 1:
            do_admin_query(msg, sfd);
            break;
        case 2:
            do_admin_modification_ui(sfd,name);
            break;
        case 3:
            do_admin_adduser_ui(sfd,name);
            break;
        case 4:
            do_admin_deluser_ui(sfd,name);
            break;
        case 5:
        do_admin_history(sfd,name);
            break;
        case 6:
            return;
        default:
            printf("输入有误请重新输入,输入任意字符清屏\n");
            while (getchar() != 10);
        }
        system("clear");
    }
}
//管理员查询
void do_admin_query(msg_t msg,int sfd)
{
    
    int choice;
    char name[40];
    while (1)
    {
        msg.cmdtype = 1;
        printf("*********************do_admin_query**************************\n");
        printf("*************************************************************\n");
        printf("*********  1:按人名查找  	2:查找所有 	3:退出***************\n");
        printf("*************************************************************\n");
        printf("请输入您的选择(数字)>>");
        scanf("%d", &choice);
        while (getchar() != 10)
            ;
        switch (choice)
        {
        case 1:

            printf("请输入你要查找的用户名>>");
            if (fgets(name, sizeof(name), stdin) == NULL)
            {
                PRINT_ERR("fgets");
                return;
            }
            name[strlen(name)-1] = 0;
            bzero(msg.buf,sizeof(msg.buf));
            strcpy(msg.st.name, name);
            do_search_ui();
            send_recv_msg(msg, sfd);
            break;

        case 2:
            bzero(msg.st.name, sizeof(msg.st.name));
            bzero(msg.buf, sizeof(msg.buf));
            strcpy(msg.buf, "all");
            do_search_ui();
            send_recv_msg(msg,sfd);
            break;
        case 3:
            return;
        }
        if(choice != 1 && choice != 2 && choice != 3)
        {
            printf("输入有误请重新输入,输入任意字符清屏\n");
        }
        else
        {
             printf("输入任意字符清屏\n");
        }
        
        while (getchar() != 10)
            ;
        system("clear");
    }
}
//接收与发送---查询专用且打印
void send_recv_msg(msg_t msg,int sfd)
{

    //查询是不是所有的信息都打印出来了需要通过buf 来判断了规定接收到的msg.buf 如果是send complete就说明全部发送完毕
    if(send(sfd,&msg,sizeof(msg_t),0) < 0 )
        {
            PRINT_ERR("send");
            return;
        }
        ssize_t ret;
        //输出查找到的结果
        while (1)
        {

            bzero(&msg, sizeof(msg_t));
            ret = recv(sfd, &msg, sizeof(msg_t), 0);

            if (ret == 0)
            {
                printf("服务器掉线\n");
                return;
            }
            else if (ret < 0)
            {
                PRINT_ERR("recv");
                return;
            }
            if (!strcmp(msg.buf, "no userinfo"))
            {
                printf("无该员工信息\n");
                break;
            }
            //printf("msg.cmdtype = %d\n",msg.cmdtype);
            if (msg.cmdtype == 0)
            {
                printf("%4d,%2d,%8s,%8s,%3d,%11s,%8s,%8s,%8s,%2d,%.1lf\n",
                       msg.st.staffno, msg.st.usertype, msg.st.name, msg.st.passward, msg.st.age,
                       msg.st.phone_number, msg.st.home_address,msg.st.postion, msg.st.date, msg.st.level, msg.st.salary);
            }
            else if(msg.cmdtype == 5)
            {
                printf("%s---%s---%s\n",msg.st.date,msg.st.name,msg.st.home_address);
            }

            if (!strcmp(msg.buf, "send complete"))
            {
                break;
            }
        }
}
//管理员修改
void do_admin_modification_ui(int sfd, char *name)
{
    //buf里第一个字节是修改的类型+空格+修改者的名字
    msg_t msg;
    bzero(&msg,sizeof(msg));

    msg.cmdtype = 2;
    strcpy(msg.buf+2,name);
    *(msg.buf+1) = ' ';
    printf("请输入需要修改的工号:");
    scanf("%d",&msg.st.staffno);
    printf("********************do_admin_modification****************\n");
    printf("*******************请输入要修改的选项********************\n");
    printf("******	1：姓名	  2：年龄	3：家庭住址   4：电话  ******\n");
    printf("******	5：职位	   6：工资  7：入职年月   8：评级  ******\n");
    printf("******	9：密码	 10：退出				          *******\n");
    printf("*********************************************************\n");
    int choice;
    printf("请输入您的选择(数字)>>");
    scanf("%d", &choice);
    while (getchar() != 10)
        ;
    switch (choice)
    {
    case 1:
        printf("请输入姓名:");
        *msg.buf = 1;
        scanf("%s", msg.st.name);
        break;
    case 2:
        printf("请输入年龄:");
        *msg.buf = 2;
        scanf("%d", &msg.st.age);
        break;
    case 3:
        printf("请输入家庭住址:");
        *msg.buf = 3;
        scanf("%s", msg.st.home_address);
        break;
    case 4:
        printf("请输入电话:");
        *msg.buf = 4;
        scanf("%s", msg.st.phone_number);
        break;
    case 5:
        printf("请输入职位:");
        *msg.buf = 5;
        scanf("%s", msg.st.postion);
        break;
    case 6:
        printf("请输入工资:");
        *msg.buf = 6;
        scanf("%lf", &msg.st.salary);
        break;
    case 7:
        printf("请输入入职年月:");
        *msg.buf = 7;
        scanf("%s", msg.st.date);
        break;
    case 8:
        printf("请输入评级:");
        *msg.buf = 8; 

        scanf("%d", &msg.st.level);
        break;
    case 9:
        printf("请输入密码:");
        *msg.buf = 9;
        scanf("%s", msg.st.passward);
        break;
    case 10:
        return;
    default:
        printf("输入有误,请重新输入\n");
        printf("输入任意键清屏\n");
        while (getchar() != 10)
            ;
        return;
    }
            while (getchar() != 10)
            ;

    if(send(sfd,&msg,sizeof(msg),0) < 0)
    {
        PRINT_ERR("send");
        return ;
    }
    bzero(&msg,sizeof(msg));
    ssize_t ret = recv(sfd,&msg,sizeof(msg),0);
    if(ret == 0)
    {
        printf("服务器下线");
    }
    else if(ret < 0)
    {
        PRINT_ERR("recv");
    }
    if(!strcmp(msg.buf,"modify success"))
    {
        printf("数据库修改成功！修改结束");
    }
    else
    {
        printf("数据库修改失败,请重试\n");   
    }
    printf("输入任意键清屏\n");
    while(getchar() != 10);
}
//管理员添加
void do_admin_adduser_ui(int sfd,char *name)
{
    msg_t msg;
    char choice;
    ssize_t ret;
    while (1)
    {
        bzero(&msg,sizeof(msg));
        strcpy(msg.buf,name);   //存一下操作用户名
        msg.cmdtype = 3;   //添加用户信息类型
        printf("********************do_admin_adduser****************\n");
        printf("***************热烈欢迎新员工***********************\n");
        while (1)
        {
            printf("请输入工号:");
            scanf("%d", &msg.st.staffno);
            while (getchar() != 10)
                ;
            printf("你输入的工号是:%d\n工号信息一旦录入无法更改,请确认输入无误!(Y/N)", msg.st.staffno);
            scanf("%c", &choice);
            while (getchar() != 10)
                ;
            if (choice == 'n' || choice == 'N')
            {
                continue;
            }
            else
            {
                break;
            }
        }
        printf("请输入用户名:");
        scanf("%s", msg.st.name);
        while (getchar() != 10)
            ;
        printf("请输入用户密码:");
        scanf("%s", msg.st.passward);
        while (getchar() != 10)
            ;
        printf("请输入年龄:");
        scanf("%d", &msg.st.age);
        while (getchar() != 10)
            ;
        printf("请输入电话:");
        scanf("%s", msg.st.phone_number);
        while (getchar() != 10)
            ;
        printf("请输入家庭住址:");
        scanf("%s", msg.st.home_address);
        while (getchar() != 10)
            ;
        printf("请输入职位:");
        scanf("%s", msg.st.postion);
        while (getchar() != 10)
            ;
        printf("请输入入职日期(格式:0000.00.00):");
        scanf("%s", msg.st.date);
        while (getchar() != 10)
            ;
        printf("请输入评级(1~5,5为最高,新员工为1):");
        scanf("%d", &msg.st.level);
        while (getchar() != 10)
            ;
        printf("请输入薪资:");
        scanf("%lf", &msg.st.salary);
        while (getchar() != 10)
            ;
        printf("是否为管理员(Y/N):");
        choice = 0;
        scanf("%c", &choice);
        if (choice == 'Y' || choice == 'y')
        {
            msg.usertype = 0;
        }
        else
        {
            msg.usertype = 1;
        }
        while (getchar() != 10)
            ;
        //发送给服务器
        if(send(sfd,&msg,sizeof(msg),0) < 0)
        {
            PRINT_ERR("send");
            break;
        }
        bzero(msg.buf,sizeof(msg.buf));
        ret = recv(sfd,&msg,sizeof(msg),0);
        if(ret  == 0)
        {
            printf("服务器掉线\n");
            break;
        }
        else if(ret < 0)
        {
            PRINT_ERR("recv");
            break;
        }
        if (!strcmp(msg.buf, "adduser success"))
        {
            printf("数据库添加成功!");
            printf("是否继续添加员工:(Y/N)");
            if (scanf("%c", &choice) == 1 && (choice == 'y' || choice == 'Y'))
            {
                continue;
            }
            else
            {
                break;
            }
        }
        else
        {
            printf("添加失败,请稍后再试\n");
           // system("clear");
            break;
        }
    }
}
//管理员删除
void do_admin_deluser_ui(int sfd,char *name)
{
    msg_t msg;
   
    bzero(&msg,sizeof(msg));
    strcpy(msg.buf,name);
    printf("********************do_admin_deluser****************\n");
    printf("请输入要删除的用户工号:");
    scanf("%d",&msg.st.staffno);
    while(getchar()!= 10);
    printf("请输入要删除的用户名:");
    scanf("%s",msg.st.name);
    while(getchar()!= 10);
    msg.usertype=msg.st.usertype = 0;
    msg.cmdtype = 4;
    send(sfd,&msg,sizeof(msg),0);
    bzero(&msg,sizeof(msg));
    recv(sfd,&msg,sizeof(msg),0);

    if(!strcmp(msg.buf,"delete success"))
    {
        printf("删除成功\n");
    }
    else
    {
        printf("删除失败，请重试\n");
    }

    printf("输入任意键清屏\n");
    while(getchar()!= 10);
    

   
}
//历史记录
void do_admin_history(int sfd,char *name)
{
    msg_t msg;
    bzero(&msg,sizeof(msg));
    msg.cmdtype = 5;
    printf("********************do_admin_history****************\n");
    send_recv_msg(msg,sfd);
    printf("%s查询历史记录结束!\n",name);
    printf("按任意键清屏\n");
    while(getchar() != 10);
    
}


//普通用户登录后的界面
void user_menu_ui(msg_t msg,int sfd,char *name)
{
    int choice;

    while (1)
    {
        
        printf("*************************user_menu***************************\n");
        printf("*************************************************************\n");
        printf("***************  1:查询    2:修改    3:退出******************\n");
        printf("*************************************************************\n");
        printf("请输入您的选择(数字)>>");
        scanf("%d", &choice);
        while (getchar() != 10)
            ;
        switch (choice)
        {
        case 1: //普通用户查询只能查询自己的信息
            msg.cmdtype = 1;   //查询类型
            do_search_ui();
            send_recv_msg(msg,sfd);
            break;
        case 2:
            msg.cmdtype = 2;   //修改类型
            do_user_modification_ui(sfd,name);
            break;
        case 3:
            return;
        default:
            printf("输入有误请重新输入,输入任意字符清屏\n");
            while (getchar() != 10)
                ;
            system("clear");
            continue;
        }

        printf("输入任意字符清屏\n");
        while (getchar() != 10)
            ;
        system("clear");
    }
}
//普通用户修改界面---
void do_user_modification_ui(int sfd,char *name)
{
    msg_t msg;
    int choice;
    bzero(&msg,sizeof(msg));

    msg.cmdtype = 2;
    strcpy(msg.buf+2,name);
    *(msg.buf+1) = ' ';
    printf("请输入需要修改的工号:");
    scanf("%d",&msg.st.staffno);
    printf("********************do_user_modification*********************\n");
    printf("************请输入要修改的选项(其他信息请联系管理员)*********\n");
    printf("**********  1:家庭住址    2:电话   3:密码  4.退出************\n");
    printf("*************************************************************\n");
    printf("请输入您的选择>>");
    scanf("%d",&choice);
    while(getchar()!= 10);
    switch(choice)
    {
        case 1:
            *msg.buf = 3;
            printf("请输入家庭住址:");
            bzero(msg.st.home_address,sizeof(msg.st.home_address));
            scanf("%s",msg.st.home_address);
            break;
        case 2:
            *msg.buf = 4;
            printf("请输入电话:");
            bzero(&msg.st.phone_number,sizeof(msg.st.phone_number));
            scanf("%s",msg.st.phone_number);
            break;
        case 3:
            *msg.buf = 9;
            printf("请输入密码:");
            bzero(msg.st.passward,sizeof(msg.st.passward));
            scanf("%s",msg.st.passward);
            break;
        case 4:
            return ;
        default:
        printf("输入有误,输入任意键清屏\n");
        return ;
    }
    while(getchar() != 10);
    msg.cmdtype = 2;
    msg.usertype = msg.st.usertype = 1;
    if(send(sfd,&msg,sizeof(msg),0) <0)
    {
        PRINT_ERR("send");
        return;
    }
    bzero(msg.buf,sizeof(msg.buf));
    recv(sfd,&msg,sizeof(msg),0);
    if(!strcmp(msg.buf,"modify success"))
    {
        printf("数据库修改成功\n");
    }
    else
    {
        printf("数据修改失败,请重试\n");
    }
}


void do_search_ui(void)
{
    printf("   工号   用户类型  姓名   密码    年龄   电话  地址    职位    入职年月   等级    工资\n");
    printf("========================================================================================\n");
}



