#ifndef QTECSTRUCT_H
#define QTECSTRUCT_H


//======qtec list============//
struct qtec_list;

struct qtec_list
{
    struct qtec_list *next;
    struct qtec_list *prev;
};



//======qtec device entry=======//
enum device_type{
    QTEC_TYPE_UNSPEC = 0,
    QTEC_TYPE_DOOR  = 1,
    QTEC_TYPE_OTHER = 2,
};



//这个结构体只是粗略的显示设备信息，不展示细节信息
struct simpleDeviceEntry
{
    struct qtec_list root;
    char deviceid[32];
    enum device_type type;
    char name[64];
    int status;
    char ieee_addr[17];
    int nw_addr;
    char version[8];
    char model[8];
    char seq[8];
};


//============door entry===============//
//suppose every door can store 10 passowrd entry and 10 fingerprint entry at max
#define DOOR_MAX_STORAGE  10

enum door_status 
{
    QTEC_DOOR_STATUS_UNSPEC = 0,
    QTEC_DOOR_STATUS_LOCKED = 1,
    QTEC_DOOR_STATUS_UNLOCKED = 2,
    QTEC_DOOR_STATUS_ALERT = 3,
    QTEC_DOOR_STATUS_OTHER = 4,
};



//suppose password support 16 chars at longest
enum password_grade
{
    QTEC_PASSWORD_GRADE_UNSPEC = 0,
    QTEC_PASSWORD_GRADE_ADMIN = 1,
    QTEC_PASSWORD_GRADE_COMMON = 2,
    QTEC_PASSWORD_GRADE_OTHER = 3,
};

struct PasswordEntry
{
    char password[16]; 
    char passwordid[32];
    enum password_grade grade;
};




struct FingerPrintEntry
{
    struct qtec_list root;
    char fingerprintid[32];
    char name[64];
    char deviceid[32];
    char userid[64];
};




//============UserEntry====

enum user_grade
{
    QTEC_USER_GRADE_UNSPEC=0,
    QTEC_USER_GRADE_ADMIN=1,
    QTEC_USER_GRADE_FAMILY=2,
    QTEC_USER_GRADE_GUEST=3,
    QTEC_USER_GRADE_OTHER=4,
};

struct UserEntry
{
    struct qtec_list root;
    char username[64];
    char userid[64];
    enum user_grade grade;
};


struct LogEntry
{
    struct qtec_list root;
    char time[64];
    int opratetype;
    char userid[64];
    char devid[32];
};

#endif
