#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <errno.h>
#include    <sys/types.h>
#include    <sys/socket.h>
#include    <sys/time.h>
#include    <sys/file.h>
#include    <netinet/in.h>
#include    <time.h>
#include    <signal.h>
#include    <mysql.h>
#include    <openssl/md5.h>
#include    <fcntl.h>

#define FILESIZE    10
#define BUFSIZE     1024

//数据库参数
#define MYSQL_USER	"G2335"
#define MYSQL_PWD	"G2335"
#define MYSQL_ADDR	"localhost"
#define MYSQL_DB	"G2335"

//服务器地址
#define SERVER_IP   "192.168.1.230"
#define SERVER_PORT 1221

//协议
#define PROTOCOL_LEN                4
#define PROTOCOL_END                "end"
#define PROTOCOL_SIGNUP             "0101"
#define PROTOCOL_LOGIN              "0102"

#define PROTOCOL_GET_FILELIST_SIZE  "0200"
#define PROTOCOL_GET_FILELIST       "0201"
#define PROTOCOL_GET_MD5            "0202"
#define PROTOCOL_GET_BINDCATALOG    "0203"

#define PROTOCOL_SET_FILELIST       "0301"
#define PROTOCOL_SET_BINDCATALOG    "0302"

#define PROTOCOL_UPLOAD             "0401"
#define PROTOCOL_DOWNLOAD           "0402"

//记号
#define MARK_NULL_CATALOG           "NOT BINDED"

//路径
#define PATH_LOGFILE	"/home/G2335/server/log/"
#define PATH_SERVERFILE "/home/G2335/server/files/"
#define PATH_LOCK	"/home/G2335/server/lock/"

//用户默认目录结构
char * UserDefaultCatalog[]=
{
"file",
"file/document",
"file/download",
"file/music",
"file/picture",
};
