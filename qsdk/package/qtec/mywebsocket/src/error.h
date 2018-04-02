#ifndef ERROR_H
#define ERROR_H
enum error_type
{
    ERROR_UNSPEC = 1,
    ERROR_MEM = 2,
    ERROR_NOUSER =3,
    ERROR_WRONGGRADE=4,
    ERR_DATA_WRONG =5,
    ERR_DB_CMD=6,
    ERROR_OTHER,
};

enum error_type global_error;

#define program_quit() \
    {                   \
        DEBUG_PRINTF("====%s======%d=====error: %d ====\n", __func__, __LINE__,global_error); \
        exit(0);                                                                             \
    }


#endif

