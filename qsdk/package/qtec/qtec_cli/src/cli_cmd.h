#ifndef __CLI_CMD_H__
#define __CLI_CMD_H__


/* Argc max counts. */
#define CMD_MAX_ARG_COUNT   50

#define CMD_MAX_WORD_LEN    500

#define CMD_INIT_MATCHVEC_SIZE 10

/* Return value of the commands. */
#define CMD_SUCCESS              0
#define CMD_WARNING              1
#define CMD_ERR_NO_MATCH         2
#define CMD_ERR_AMBIGUOUS        3
#define CMD_ERR_INCOMPLETE       4
#define CMD_ERR_EXEED_ARGC_MAX   5
#define CMD_ERR_NOTHING_TODO     6
#define CMD_COMPLETE_FULL_MATCH  7
#define CMD_COMPLETE_MATCH       8
#define CMD_COMPLETE_LIST_MATCH  9


/*√¸¡Ó√Ë ˆΩ·ππ. */
typedef struct cmd_desc
{
    char *cmd;
    char *str;
} CMD_DESC_T;


typedef struct
{
    UTIL_VECTOR cmd_desc_vec;
    UINT32 fixed_cmd_count;
} CMD_ITEM_T;


extern UTIL_VECTOR sg_cmdvec;


void cmd_describe_command(VTY_T *vty,
                          UTIL_VECTOR line_vec,
                          UTIL_VECTOR match_vec);
UINT32 cmd_complete_command(VTY_T *vty,
                            UTIL_VECTOR line_vec,
                            char match[][CMD_MAX_WORD_LEN],
                            UINT32 max_match_count,
                            UINT32 *match_len);
UINT32 cmd_execute_command(VTY_T *vty, UTIL_VECTOR line_vec);

char *cmd_prompt(CLI_NODE_ID nodeid);
CLI_NODE_ID cmd_find_node(CLI_NODE_ID rootNodeId, const char *path);
UTIL_VECTOR cmd_make_strvec(char *cmd);
void cmd_free_strvec(UTIL_VECTOR strvec);
VOS_RET_E cmd_init();


#endif /* __CLI_CMD_H__ */
