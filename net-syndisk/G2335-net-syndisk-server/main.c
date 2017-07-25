#include    "define.h"
#include     <string.h>

MYSQL mysql;

void error(char *info)
{
    printf("%s: %s(errno: %d)\n",info,strerror(errno),errno);
}

int myrecv(int sockfd, char *buf, int messagelen)
{
    int len = 0;

    while (len < messagelen)
    {
        len += recv(sockfd, buf + len, messagelen - len, 0);
    }

    return messagelen ;
}

int mysend(int sockfd, char *buf, int messagelen)
{
    int len = 0;

    while (len < messagelen)
    {
        len += send(sockfd, buf + len, messagelen - len, 0);
    }

    return messagelen ;
}

int SetBindCatalog(int sockfd)
{
    char    query[200] ;
    char    username[17]   = {'\0'};
    char    catalog[128]   = {'\0'};

    recv(sockfd, username, 16, 0);
    recv(sockfd, catalog, 128, 0);

    sprintf(query, "update User set catalog='%s' where username='%s'", catalog, username);
    if (mysql_real_query(&mysql, query, strlen(query)) != 0)
    {
        printf("error\n");
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return 0;
    }
    return 1;
}

int SetFileList(int sockfd)
{
    int     filelist_lock ;
    char    query[200] ;
    char    filename[200];
    char    username[17]   = {'\0'};
    int     Userid ;

    recv(sockfd, username, 16, 0);

    //给filelist加锁
    sprintf(filename, "%s%s.lock", PATH_LOCK, username);
    filelist_lock = open( filename, O_RDONLY );

    if (filelist_lock == 0)
        printf("cannot open the file");
    if (flock (filelist_lock, LOCK_EX) == 0)
        printf("I lock the file\n");

    //查找userid
    sprintf(query, "select userid from User where username='%s'", username);
    if (mysql_real_query(&mysql, query, strlen(query)) != 0)
    {
        printf("error\n");
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return 0;
    }

    MYSQL_RES * res =  mysql_store_result(&mysql);
    if ((unsigned long)mysql_num_rows(res))
    {
        MYSQL_ROW row = mysql_fetch_row(res);
        Userid = atoi(row[0]);
    }
    else
        return 0;

    //循环接收文件信息，收到END退出
    while(1)
    {
        char path[128]      = {'\0'};
        char isFile[2]      = {'\0'};
        char updateTime[20] = {'\0'};
        char md5[33]        = {'\0'};

        myrecv(sockfd, path, 128);
        //        printf("path%s\n", path);
        if (!strcmp( path, PROTOCOL_END))
            break;
        myrecv(sockfd, isFile, 1);
        myrecv(sockfd, updateTime, 19);
        myrecv(sockfd, md5, 32);
        //        printf("md5:%s\n", md5);
        sprintf(query, "insert into File values(%d, '%s', %d, '%s', '%s')", Userid, path, atoi(isFile), updateTime, md5);
        mysql_real_query(&mysql, query, strlen(query));
    }

    //写完数据库后解锁
    close(filelist_lock);
}

int GetFileSize(const char *path)
{
    unsigned long filesize = -1;
    struct stat statbuff;
    if(stat(path, &statbuff) < 0){
        return filesize;
    }else{
        filesize = statbuff.st_size;
    }
    return filesize;
}

int GetFile(int sockfd)
{
    char    query[200] ;

    //循环接收文件信息，收到END退出
    while(1)
    {
        char path[128]      = {'\0'};
        char isFile[2]      = {'\0'};
        char updateTime[20] = {'\0'};
        char md5[33]        = {'\0'};
        char filename[100];
        char filesize[11]   = {'\0'};
        char buf[BUFSIZE + 1] = {'\0'};
        int  recvlen = 0;
        int  size ;

        myrecv(sockfd, path, 128);
        if (!strcmp(path, PROTOCOL_END))
            break ;
        recv(sockfd, isFile, 1, 0);
        recv(sockfd, updateTime, 19, 0);
        recv(sockfd, md5, 32, 0);

        sprintf(filename, "%s%s.txt", PATH_SERVERFILE ,md5);
        FILE* fp = fopen(filename, "wb");
        if (fp == NULL)
        {
            error("cannot open the file");
            return 0;
        }

        recv(sockfd, filesize, FILESIZE, 0);
        size = atoi(filesize);

        while(recvlen < size)
        {
            recvlen += myrecv(sockfd, buf, BUFSIZE);

            if (recvlen > size)
                fwrite(buf, 1, size % BUFSIZE, fp);
            else
                fwrite(buf, 1, BUFSIZE, fp);
        }

        fclose(fp);
        printf("fileget done\n");
        //将真实文件的md5值写入数据库
        sprintf(query, "insert into Serverfile values('%s')", md5);
        if (mysql_real_query(&mysql, query, strlen(query)) != 0)
        {
            printf("error\n");
            printf("mysql_real_query(): %s\n", mysql_error(&mysql));
            return 0;
        }
    }
}

//将用户绑定目录发送给客户端
int SendBindCatalog(int sockfd)
{
    char    query[100] ;


    char username[17] = {'\0'};
    recv(sockfd, username, 16, 0);

    sprintf(query, "select catalog from User where username = '%s'", username);
    if (mysql_real_query(&mysql, query, strlen(query)) != 0)
    {
        printf("error\n");
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return 0;
    }
    MYSQL_RES * res =  mysql_store_result(&mysql);
    if ((unsigned long)mysql_num_rows(res))
    {
        MYSQL_ROW row ;
        row = mysql_fetch_row(res);
        send(sockfd, row[0], 128, 0);
        return 1;
    }
    return 0;
}

//将服务器真实文件MD5发送给客户端
int SendMd5s(int sockfd)
{
    char    query[100] ;

    sprintf(query, "select md5 from Serverfile");
    if (mysql_real_query(&mysql, query, strlen(query)) != 0)
    {
        printf("error\n");
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return 0;
    }

    MYSQL_RES * res =  mysql_store_result(&mysql);
    if ((unsigned long)mysql_num_rows(res))
    {
        MYSQL_ROW row ;

        while(row = mysql_fetch_row(res))
        {
            send(sockfd, row[0], strlen(row[0]), 0);
        }
    }
    send(sockfd, PROTOCOL_END, 32, 0);
    return 0;
}

//将默认目录发送给客户端
int SendFileList(char *username, int connectfd)
{
    char    query[200] ;
    char    filename[200];
    int     filelist_lock;

    //给filelist加锁
    sprintf(filename, "%s%s.lock", PATH_LOCK, username);
    filelist_lock = open( filename, O_RDONLY );
    flock (filelist_lock, LOCK_EX);

    sprintf(query, "select updateTime,isFile,md5,path from File,User where User.username='%s' and File.userid=User.userid", username);
    if (mysql_real_query(&mysql, query, strlen(query)) != 0)
    {
        printf("error\n");
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return 0;
    }

    MYSQL_RES * res =  mysql_store_result(&mysql);
    if ((unsigned long)mysql_num_rows(res))
    {
        MYSQL_ROW row ;

        while(row = mysql_fetch_row(res))
        {
            send(connectfd, row[0], 19, 0);
            send(connectfd, row[1], 1, 0);
            send(connectfd, row[2], 32, 0);
            send(connectfd, row[3], 128, 0);
        }
    }
    send(connectfd, PROTOCOL_END, 19, 0);
    mysql_close(&mysql) ;
    close(filelist_lock);
    return 0;
}

//将同步目录文件总数发给客户端
int SendFileListSize(int sockfd)
{
    char                query[200];
    char                buf[FILESIZE + 1];
    char                username[17] = {'\0'};
    char                filename[200];
    unsigned long       size;
    int                 filelist_lock ;
    int                 Userid;

    recv(sockfd, username, 16, 0);

    //查找userid
    sprintf(query, "select userid from User where username='%s'", username);
    if (mysql_real_query(&mysql, query, strlen(query)) != 0)
    {
        printf("error\n");
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return 0;
    }
    MYSQL_RES * res =  mysql_store_result(&mysql);
    if ((unsigned long)mysql_num_rows(res))
    {
        MYSQL_ROW row = mysql_fetch_row(res);
        Userid = atoi(row[0]);
    }
    else
        return 0;

    //给filelist加锁
    sprintf(filename, "%s%s.lock", PATH_LOCK, username);
    if ( (filelist_lock = open( filename, O_RDONLY )) == 0 )
        printf("cannot open the file");

    flock (filelist_lock, LOCK_EX) ;

    sprintf(query, "select * from File where userid = %d", Userid);
    if (mysql_real_query(&mysql, query, strlen(query)) != 0)
    {
        printf("error\n");
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return 0;
    }

    res =  mysql_store_result(&mysql);
    size = (unsigned long)mysql_num_rows(res) ;
    sprintf(buf, "%d", size);
    send(sockfd, buf, FILESIZE, 0);

    close(filelist_lock);
    return 1;
}

//发送文件给客户端
int SendFile(int sockfd)
{
    char    filename[128];
    int     filesize ;
    FILE    *file;
    while(1)
    {
        char    md5[33]= {'\0'};

        myrecv(sockfd, md5, 32);
        printf("md5:%s\n", md5);
        if (!strcmp(md5 , PROTOCOL_END))
            break;

        sprintf(filename, "%s%s.txt", PATH_SERVERFILE, md5);
        filesize = GetFileSize(filename) ;
        file = NULL;
        file = fopen(filename, "r");
        if (file == NULL)
        {
            error("cannot open the server file");
            return 0;
        }

        char    buf[BUFSIZE + 1] ;
        char    sizebuf[FILESIZE + 1] ;
        int     len ;

        sprintf(sizebuf, "%d", filesize);
        send(sockfd, sizebuf, FILESIZE, 0);

        printf("send file:%s\n", filename);

        while ( (len = fread(buf, 1, BUFSIZE, file)) > 0)
        {
            buf[len] = '\0';
            send(sockfd, buf, BUFSIZE, 0);
        }
        printf("send file done\n\n");
        fclose(file);
    }
    return 1;
}

//创建登录日志
int CreateLoginLogFile(char* username, int flags)
{
    char filename[128] ;
    char loginfo[128];
    char timestr[20];

    struct timeval tv;
    struct tm* ptm;
    gettimeofday(&tv, NULL);
    ptm = localtime(&tv.tv_sec);
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", ptm);

    sprintf(filename, "%s%s.txt", PATH_LOGFILE , username);
    FILE* file = fopen(filename, "a") ;

    switch(flags)
    {
    case -1:
        sprintf(loginfo, "Last login: %s\r\nLogin failed:user doesn't exist\r\n\r\n", timestr);
        break;
    case 0:
        sprintf(loginfo, "Last login: %s\r\nLogin failed:incorrected username or password\r\n\r\n", timestr);
        break;
    case 1:
        sprintf(loginfo, "Last login: %s\r\nLogin success\r\n\r\n", timestr);
        break;
    }

    fwrite(loginfo, strlen(loginfo), 1, file);
    fclose(file);
    return 1 ;
}

int CreateLockFile(char *username)
{
    char filename[128] ;
    int  lockfp ;

    sprintf(filename, "%s%s.lock", PATH_LOCK, username);
    lockfp = open(filename, O_RDONLY|O_CREAT);

    if (lockfp == -1)
    {
        error("cannot create lock file");
        return 0;
    }
    close(lockfp);
    return 1;
}

//为新注册的用户创建默认目录
int CreateUserDefaultCatalog(char *username)
{
    char    query[100] ;
    int     Userid ;

    //查找userid
    sprintf(query, "select userid from User where username='%s'", username);
    if (mysql_real_query(&mysql, query, strlen(query)) != 0)
    {
        printf("error\n");
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return 0;
    }
    MYSQL_RES * res =  mysql_store_result(&mysql);
    if ((unsigned long)mysql_num_rows(res))
    {
        MYSQL_ROW row = mysql_fetch_row(res);
        Userid = atoi(row[0]);
    }
    else
        return 0;

    //    插入File表
    sprintf(query, "insert into File values(%d, '%s', 0, now(), '00000000000000000000000000000000')", Userid, username);
    if (mysql_real_query(&mysql, query, strlen(query)) != 0)
    {
        printf("error\n");
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return 0;
    }

    int  i=0 ;
    while(1)
    {
        sprintf(query, "insert into File values(%d, '%s/%s', 0, now(), '00000000000000000000000000000000')", Userid, username, UserDefaultCatalog[i]);
        if (mysql_real_query(&mysql, query, strlen(query)) != 0)
        {
            printf("error\n");
            printf("mysql_real_query(): %s\n", mysql_error(&mysql));
            return 0;
        }
        if (++i == 5) break;
    }
    return 1;
}

//对字符串进行MD5加密，返回32位字符串，结果存储在target里
void MD5String(char *source, int sourceLen, char *target)
{
    char tmp[3];
    char target_tmp[16];
    int  i;
    MD5(source, sourceLen, target_tmp);
    for (i=0; i<16; i++)
    {
        sprintf(tmp,"%2.2x",(unsigned char)target_tmp[i]);
        strcat(target,tmp);
    }
}

int MD5File(char *filename, char *target)
{
    MD5_CTX ctx;
    unsigned char outmd[16];
    char    temp[3];
    char    buf[1024];
    int     len=0;
    int     i;

    FILE * fp=NULL;
    fp=fopen(filename,"rb");
    if(fp==NULL)
    {
        printf("Can't open file\n");
        return 0;
    }

    MD5_Init(&ctx);
    while((len=fread(buf,1,1024,fp))>0)
    {
        MD5_Update(&ctx,buf,len);
        memset(buf,0,sizeof(buf));
    }

    MD5_Final(outmd,&ctx);
    for(i=0;i<16;i<i++)
    {
        sprintf(temp, "%02X",outmd[i]);
        strcat(target, temp);
    }
    return 1;
}


int Login(char *username, char* pwd)
{
    char    query[100] ;
    char    pwd_md5[33];

    //查询username
    sprintf(query, "select password from User where username='%s'", username);
    if (mysql_real_query(&mysql, query, strlen(query)) != 0)
    {
        printf("error\n");
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return 0;
    }
    //结果集
    MYSQL_RES * res =  mysql_store_result(&mysql);
    if ((unsigned long)mysql_num_rows(res))
    {
        MYSQL_ROW row = mysql_fetch_row(res);
        MD5String(pwd, strlen(pwd), pwd_md5);
        return !strcmp(row[0], pwd_md5);
    }
    else
        return -1;
}

int Signup(char *username, char* pwd)
{
    char    query[100] ;
    char pwd_md5[33];

    //查询username
    sprintf(query, "select * from User where username='%s'", username);
    if (mysql_real_query(&mysql, query, strlen(query)) != 0)
    {
        printf("error\n");
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return 0;
    }
    //结果集
    MYSQL_RES * res =  mysql_store_result(&mysql);
    if ((unsigned long)mysql_num_rows(res) == 0)
    {
        //将新用户信息插入数据库
        MD5String(pwd, strlen(pwd), pwd_md5);
        sprintf(query, "insert into User values(NULL, '%s', '%s', 'NOT BINDED')", username, pwd_md5);
        if (mysql_real_query(&mysql, query, strlen(query) ) != 0)
        {
            printf("error\n");
            printf("mysql_real_query(): %s\n", mysql_error(&mysql));
            return 0;
        }
        //生成用户文件目录
        CreateLockFile(username);
        return CreateUserDefaultCatalog(username);
    }
    else
    {
        return 0;
    }
}

int client(int connectfd)
{
    //分配和初始化MYSQL对象
    if (NULL == mysql_init(&mysql)) {
        printf("mysql_init(): %s\n", mysql_error(&mysql));
        return 0;
    }
    //尝试与运行在主机上的MySQL数据库引擎建立连接
    if  (NULL == mysql_real_connect(&mysql,
                                    MYSQL_ADDR,
                                    MYSQL_USER,
                                    MYSQL_PWD,
                                    MYSQL_DB,
                                    0,
                                    NULL,
                                    0)) {
        printf("mysql_real_connect(): %s\n", mysql_error(&mysql));
        return 0;
    }

    while(1)
    {
        char buf[BUFSIZE + 1];

        int n = recv(connectfd, buf, PROTOCOL_LEN, 0);
        if (n <= 0) break ;
        buf[n] = '\0';

        if (!strcmp(buf, PROTOCOL_LOGIN))
        {
            printf("start login\n");
            char username[17] = {'\0'};
            char password[17] = {'\0'};
            n = recv(connectfd, username, 16, 0);
            n = recv(connectfd, password, 16, 0);

            int flag = Login(username, password);
            if (flag > 0)
                send(connectfd, "Login Success", BUFSIZE, 0);
            else
                if (flag < 0)
                    send(connectfd, "Login failed:user doesn't exist",
                         BUFSIZE, 0);
                else
                    if (!flag)
                        send(connectfd, "Login failed:Incorrected username or password",
                             BUFSIZE, 0);
            CreateLoginLogFile(username, flag);
            printf("end login\n\n");
            break ;
        }
        else
            if (!strcmp(buf, PROTOCOL_SIGNUP))
            {
                printf("start signup\n");
                char username[17] = {'\0'};
                char password[17] = {'\0'};
                n = recv(connectfd, username, 16, 0);
                n = recv(connectfd, password, 16, 0);

                if (Signup(username, password))
                {
                    send(connectfd, "Sign up Success", BUFSIZE, 0);
                }
                else
                {
                    send(connectfd, "Sign up failed:this account has already been signed up",
                         BUFSIZE, 0);
                }
                printf("end signup\n\n");
                break;
            }
            else
                if (!strcmp(buf, PROTOCOL_GET_FILELIST_SIZE))
                {
                    SendFileListSize(connectfd) ;
                    break ;
                }
                else
                    if (!strcmp(buf, PROTOCOL_GET_FILELIST))
                    {
                        printf("getfilelist start \n");
                        char username[16];
                        n = recv(connectfd, username, 16, 0);
                        username[n] = '\0';
                        SendFileList(username, connectfd);
                        printf("getfilelist end \n\n");
                        break ;
                    }
                    else
                        if (!strcmp(buf, PROTOCOL_GET_MD5))
                        {
                            printf("get md5 start\n");
                            SendMd5s(connectfd);
                            printf("get md5 end\n\n");
                            break ;
                        }
                        else
                            if (!strcmp(buf, PROTOCOL_GET_BINDCATALOG))
                            {
                                printf("get catalog start\n");
                                SendBindCatalog(connectfd);
                                printf("get catalog end\n\n");

                                break ;
                            }
                            else
                                if (!strcmp(buf, PROTOCOL_UPLOAD))
                                {
                                    printf("upload start\n");
                                    GetFile(connectfd) ;
                                    printf("upload end\n\n");
                                    break ;
                                }
                                else
                                    if (!strcmp(buf, PROTOCOL_DOWNLOAD))
                                    {
                                        printf("download start\n");
                                        SendFile(connectfd) ;
                                        printf("download end\n\n");
                                        break;
                                    }
                                    else
                                        if (!strcmp (buf, PROTOCOL_SET_FILELIST))
                                        {
                                            printf("setfilelist start\n");
                                            SetFileList(connectfd);
                                            printf("setfilelist end\n\n");
                                            break;
                                        }
                                        else
                                            if (!strcmp (buf, PROTOCOL_SET_BINDCATALOG))
                                            {
                                                printf("set bindcalog start\n");
                                                SetBindCatalog(connectfd);
                                                printf("set bindcalog end\n\n");
                                                break;
                                            }
                                            else
                                                if (!strcmp(buf, PROTOCOL_END))
                                                {
                                                    break ;
                                                }
    }
    close(connectfd);
    mysql_close(&mysql);
    return 1;
}

int initServer()
{
    int s;
    int connectfd;
    int opt;
    pid_t pid ;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        error("create s error");
        exit(-1);
    }

    opt=1;
    if (setsockopt(s, SOL_SOCKET,  SO_REUSEADDR , (char *)&opt, sizeof(opt)) < 0)
    {
        error("set socket opt error");
        close(s);
        exit(-1);
    }

    struct sockaddr_in servaddr ;
    memset(&servaddr,0,sizeof(&servaddr));
    servaddr.sin_family         = AF_INET;
    servaddr.sin_addr.s_addr    = htonl(INADDR_ANY);
    servaddr.sin_port           = htons(SERVER_PORT);

    if (bind(s, (struct sockaddr *)&servaddr, sizeof(struct sockaddr))<0) {
        error("bind error");
        close(s);
        exit(-2);
    }
    if (listen(s,  200)<0) {
        error("listen error");
        close(s);
        exit(-1);
    }

    while(1)
    {
        if ( (connectfd = accept(s,(struct sockaddr*)NULL,NULL)) == -1 )
        {
            error("connect error");
            sleep(5);
            continue;
        }
        pid = fork();
        if ( pid==0 || pid==-1)	break;
    }

    if (pid == 0)
    {
        client(connectfd);
    }
    else
        if (pid == -1)
        {
            error("fork error");
            close(s);
            exit(-1);
        }
        else
        {
            close(s);
            exit(0);
        }
}

int main()
{
    //    daemon(0,1);
    signal(SIGCHLD,SIG_IGN);
    initServer();
}
