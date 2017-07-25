#ifndef DEFINE_H
#define DEFINE_H

//大小
#define BUFSIZE     1024
#define FILESIZE    10

//服务器地址
#define SERVER_IP   "10.60.102.252"
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

//路径
#define PATH_CONF "D:\\Qt\\confg\\"
#define PATH_LOG  "D:\\Qt\\log\\"

//图标
#define ICON_DIR QIcon(":/icon/dir.ico")
#define ICON_FILE QIcon(":/icon/file.ico")

//记号
#define MARK_NULL_CATALOG           "NOT BINDED"

#endif // DEFINE_H
