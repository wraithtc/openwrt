#ifndef __CLI_VTY_H__
#define __CLI_VTY_H__


typedef UINT32 CLI_LEVEL;

#define CLI_LEVEL_NONE              ((CLI_LEVEL)0)
#define CLI_LEVEL_PUBLIC            ((CLI_LEVEL)1)
#define CLI_LEVEL_RD                ((CLI_LEVEL)2)
#define CLI_LEVEL_DEBUG             ((CLI_LEVEL)3)
#define CLI_LEVEL_MANUFACTORY       ((CLI_LEVEL)4)
#define CLI_LEVEL_GUEST             ((CLI_LEVEL)5) 

typedef UINT32 CLI_NODE_ID;

#define INVALID_NODE                ((CLI_NODE_ID)0)
#define ROOT_NODE                   ((CLI_NODE_ID)1)
#define USER_NODE                   ((CLI_NODE_ID)2)
#define MANUFACTORY_NODE            ((CLI_NODE_ID)3)

#define CLI_PROMPT_HOST              "3CARETEC"
#define CLI_PROMPT_END               "# "
#define CLI_PROMPT_GUEST_END         "> "

#define VTY_MAXHIST   20


/* VTY struct. */
typedef struct vty
{
    UINT32 time_stamp;

    /* Is this vty connect to file or not */
    enum
    {
        VTY_TERM, VTY_FILE, VTY_SHELL, VTY_SHELL_SERV
    } type;

    /* Node status of this vty */
    CLI_NODE_ID node;

    /* Privilege level of this vty. */
    CLI_LEVEL privilege;

    /* Output buffer. */
    struct buffer *obuf;

    /* Command input buffer */
    char *buf;

    /* Command prefix input buffer */
    char *lazy_buf;

    /* Command cursor point */
    int cp;

    /* Command length */
    int length;

    /* Command max length. */
    int max;

    /* Histry of command */
    char *hist[VTY_MAXHIST];

    /* History lookup current point */
    int hp;

    /* History insert end point */
    int hindex;

    /* For current referencing point of interface, route-map,
    access-list etc... */
    void *index;

    /* For multiple level index treatment such as key chain and key. */
    void *index_sub;

    /* For escape character. */
    unsigned char escape;

    /* Current vty status. */
    enum
    {
        VTY_NORMAL, VTY_CLOSE, VTY_MORE, VTY_START, VTY_CONTINUE, VTY_IOREDIRECT
    } status;

    /* Window width/height. */
    int width;
    int height;

    int scroll_one;

    /* Current executing function pointer. */
    int (*func)(struct vty *, void *);

    /* Terminal monitor. */
    int monitor;

    /* In configure mode. */
    int config;

    /* vty's connection's user name */
    char *user_name;

    int (*action_func)(struct vty *vty);
    int disableOutput;
    /* function that called by confirm action */

    /* vty's interface number */
    int port;

    /* vty's vlan id */
    int vid;
} VTY_T;


int vty_read(VTY_T *vty, const char *cmdLine);
void vty_init_term(void);
void vty_exit_term(void);
UINT32 vty_get_win_width(void);
VTY_T *vty_create(int id);
void vty_free(VTY_T *vty);


#endif /* __CLI_VTY_H__ */
