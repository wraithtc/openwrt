#include "fwk.h"
#include "cli_vty.h"
#include "cli_cmd.h"
#include "cli_util.h"


#define CMD_VECTOR_DEFAULT_SIZE   10

#define CMD_VARIABLE(S)   (((strchr(S,'-') == NULL)) && ((S[0]) == '<'))
#define CMD_RANGE(S)	  (((strchr(S,'-') != NULL)) && ((S[0]) == '<'))
#define CMD_IPV4(S)	   ((strcmp ((S), "A.B.C.D") == 0))
#define CMD_IPV4_PREFIX(S) ((strcmp ((S), "A.B.C.D/M") == 0))
#define CMD_RUNTIME(S)    ((S[0]) == '$')

#define CMD_PROMPT_SIZE                    (32)


/* Command UTIL_VECTOR which includes some level of command lists. */
UTIL_VECTOR sg_cmdvec;


UBOOL8 cmd_check_element_node(CMD_ELEMENT_T *element, VTY_T *vty)
{
    UBOOL8 ret = FALSE;
    int i;
    CMD_NODE_T *node = NULL;
    UTIL_VECTOR cmdVec;

    node = UTIL_VectorSlot(sg_cmdvec, element->node);
    cmdVec = node->cmd_vector;

    for (i = 0; i < UTIL_VectorMax(cmdVec); i++)
    {
        element = UTIL_VectorSlot(cmdVec, i);

        if (NULL == element->power_func && vty->privilege >= element->level)
        {
            return TRUE;
        }
        else if (element->power_func && cli_handleSelf == element->power_func)
        {
            ret = cmd_check_element_node(element, vty);
        }
        else
        {
            continue;
        }
    }
    return ret;
}


/* Fetch next description.  Used in cmd_make_descvec(). */
static char *cmd_desc_str(char **desc)
{
    UINT32 len;
    char *cp;
    char *start;
    char *token;

    cp =  *desc;

    if (cp == NULL)
    {
        return NULL;
    }

    while (isspace((int) *cp) &&  *cp != '\0')
    {
        cp++;
    }

    if (*cp == '\0')
    {
        return NULL;
    }

    start = cp;

    while (!(*cp == '\r' ||  *cp == '\n') &&  *cp != '\0')
    {
        cp++;
    }

    len = cp - start;
    token = VOS_MALLOC(len + 1);
    memcpy(token, start, len);
    *(token + len) = '\0';

    *desc = cp;

    return token;
}


static CMD_ITEM_T * cmd_make_item(void)
{
    CMD_ITEM_T *cmd_item;
    UTIL_VECTOR cmd_desc_vec;

    cmd_item = VOS_MALLOC(sizeof(CMD_ITEM_T));
    if (NULL == cmd_item)
    {
        return NULL;
    }

    cmd_desc_vec = UTIL_VectorInit(1);
    if (NULL == cmd_desc_vec)
    {
        VOS_FREE(cmd_item);
        return NULL;
    }

    memset(cmd_item, 0, sizeof(CMD_ITEM_T));
    cmd_item->cmd_desc_vec = cmd_desc_vec;
    cmd_item->fixed_cmd_count = UTIL_VectorMax(cmd_desc_vec);

    return cmd_item;
}


static UTIL_VECTOR cmd_make_descvec(char *cmd, char *desc)
{
    char *sp;
    char *token;
    char *cp;
    char *dp;
    UINT32 len;
    UBOOL8 multiple;
    UTIL_VECTOR allvec;
    CMD_ITEM_T *cmd_item = NULL;
    CMD_DESC_T *cmddesc = NULL;

    if (cmd == NULL)
    {
        return NULL;
    }

    cp = cmd;
    dp = desc;
    multiple = FALSE;

    allvec = UTIL_VectorInit(CMD_VECTOR_DEFAULT_SIZE);

    while (1)
    {
        while (isspace((int) *cp) &&  *cp != '\0')
        {
            cp++;
        }

        if (*cp == '(')
        {
            multiple = 1;
            cp++;
        }
        if (*cp == ')')
        {
            multiple = 0;
            cp++;
        }
        if (*cp == '|')
        {
            if (!multiple)
            {
                fprintf(stderr, "Command parse error!: %s\r\n", cmd);
                exit(1);
            }
            cp++;
        }

        while (isspace((int) *cp) &&  *cp != '\0')
        {
            cp++;
        }

        if (*cp == '(')
        {
            multiple = 1;
            cp++;
        }

        if (*cp == '\0')
        {
            return allvec;
        }

        sp = cp;

        while (!(isspace((int) *cp) ||  *cp == ')' ||  *cp == '|') &&  *cp != '\0')
        {
            cp++;
        }

        len = cp - sp;

        token = VOS_MALLOC(len + 1);
        memcpy(token, sp, len);
        *(token + len) = '\0';

        cmddesc = VOS_MALLOC(sizeof(CMD_DESC_T));
        cmddesc->cmd = token;
        cmddesc->str = cmd_desc_str(&dp);

        if (CMD_VARIABLE(token) || CMD_RANGE(token) || CMD_IPV4(token) || CMD_IPV4_PREFIX(token))
        {
            cmddesc->cmd = token;
        }
        else
        {
            util_strToLower(token);
            cmddesc->cmd = token;
        }

        if (multiple)
        {
            if (multiple == 1)
            {
                cmd_item = cmd_make_item();
                UTIL_VectorSet(allvec, cmd_item);
            }
            multiple++;
        }
        else
        {
            cmd_item = cmd_make_item();
            UTIL_VectorSet(allvec, cmd_item);
        }

        UTIL_VectorSet(cmd_item->cmd_desc_vec, cmddesc);
        cmd_item->fixed_cmd_count = UTIL_VectorMax(cmd_item->cmd_desc_vec);
    }

    return allvec;
}


static UTIL_VECTOR cmd_node_vector(CLI_NODE_ID nodeid)
{
    CMD_NODE_T *node;

    node = UTIL_VectorSlot(sg_cmdvec, nodeid);
    return node->cmd_vector;
}


static UBOOL8 cmd_ipv4_match(char *str)
{
    UINT32 dots = 0;
    UINT32 nums = 0;
    char *sp;
    char buf[4];

    if (str == NULL)
    {
        return FALSE;
    }

    while (1)
    {
        memset(buf, 0, sizeof(buf));
        sp = str;
        while (*str != '\0')
        {
            if (*str == '.')
            {
                if (dots >= 3)
                {
                    return FALSE;
                }

                if (*(str + 1) == '.')
                {
                    return FALSE;
                }

                if (*(str + 1) == '\0')
                {
                    return FALSE;
                }

                dots++;
                break;
            }
            if (!isdigit((int) *str))
            {
                return FALSE;
            }

            str++;
        }

        if (str - sp > 3)
        {
            return FALSE;
        }

        strncpy(buf, sp, str - sp);
        if (atoi(buf) > 255)
        {
            return FALSE;
        }

        nums++;

        if (*str == '\0')
        {
            break;
        }

        str++;
    }

    if (nums < 4)
    {
        return FALSE;
    }

    return TRUE;
}


static UBOOL8 cmd_ipv4_prefix_match(char *str)
{
    UINT32 dots = 0;
    char *sp;
    char buf[4];

    if (str == NULL)
    {
        return FALSE;
    }

    while (1)
    {
        memset(buf, 0, sizeof(buf));
        sp = str;
        while (*str != '\0' &&  *str != '/')
        {
            if (*str == '.')
            {
                if (dots == 3)
                {
                    return FALSE;
                }

                if (*(str + 1) == '.' || *(str + 1) == '/')
                {
                    return FALSE;
                }

                if (*(str + 1) == '\0')
                {
                    return FALSE;
                }

                dots++;
                break;
            }

            if (!isdigit((int) *str))
            {
                return FALSE;
            }

            str++;
        }

        if (str - sp > 3)
        {
            return FALSE;
        }

        strncpy(buf, sp, str - sp);
        if (atoi(buf) > 255)
        {
            return FALSE;
        }

        if (dots == 3)
        {
            if (*str == '/')
            {
                if (*(str + 1) == '\0')
                {
                    return FALSE;
                }

                str++;
                break;
            }
            else if (*str == '\0')
            {
                return FALSE;
            }
        }

        if (*str == '\0')
        {
            return FALSE;
        }

        str++;
    }

    sp = str;
    while (*str != '\0')
    {
        if (!isdigit((int) *str))
        {
            return FALSE;
        }

        str++;
    }

    if (atoi(sp) > 32)
    {
        return FALSE;
    }

    return TRUE;
}


static UBOOL8 cmd_range_match(char *range, char *str)
{
    char *p;
    char *endptr = NULL;
    char buf[11];
    UINT32 min;
    UINT32 max;
    UINT32 val;

    if (str == NULL)
    {
        return FALSE;
    }

    val = strtoul(str, &endptr, 10);
    if (*endptr != '\0')
    {
        return FALSE;
    }

    range++;
    p = strchr(range, '-');
    if (p == NULL)
    {
        return FALSE;
    }
    if (p - range > sizeof(buf) - 1)
    {
        return FALSE;
    }
    strncpy(buf, range, p - range);
    buf[p - range] = '\0';
    min = strtoul(buf, &endptr, 10);
    if (*endptr != '\0')
    {
        return FALSE;
    }

    range = p + 1;
    p = strchr(range, '>');
    if (p == NULL)
    {
        return FALSE;
    }
    if (p - range > sizeof(buf) - 1)
    {
        return FALSE;
    }
    strncpy(buf, range, p - range);
    buf[p - range] = '\0';
    max = strtoul(buf, &endptr, 10);
    if (*endptr != '\0')
    {
        return FALSE;
    }

    if (val < min || val > max)
    {
        return FALSE;
    }

    return TRUE;
}


/* If src matches dst return dst string, otherwise return NULL */
static char *cmd_entry_function(char *src, char *dst)
{
    if (CMD_VARIABLE(dst) || CMD_IPV4(dst) || CMD_IPV4_PREFIX(dst) || CMD_RANGE(dst) || CMD_RUNTIME(dst))
    {
        return NULL;
    }

    /* In case of 'command \t', given src is NULL string. */
    if (src == NULL)
    {
        return dst;
    }

    /* Matched with input string. */
    if (0 == strncasecmp(src, dst, strlen(src)))
    {
        return dst;
    }

    return NULL;
}


/* Compare string to description UTIL_VECTOR.  If there is same string
   return 1 else return 0. */
static UBOOL8 cmd_desc_unique_string(UTIL_VECTOR descvec, char *cmd)
{
    UINT32 i;
    CMD_DESC_T *desc;

    for (i = 0; i < UTIL_VectorMax(descvec); i++)
    {
        desc = UTIL_VectorSlot(descvec, i);
        if (desc != NULL)
        {
            if (strcasecmp(desc->cmd, cmd) == 0)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}


/* Check LCD of matched command. */
static UINT32 cmd_lcd(char match[][CMD_MAX_WORD_LEN], UINT32 count)
{
    UINT32 i;
    UINT32 j;
    UINT32 lcd = 0;
    char *s1,  *s2;
    char c1, c2;

    if (count < 2)
    {
        return 0;
    }

    for (i = 1; i < count; i++)
    {
        s1 = match[i - 1];
        s2 = match[i];

        for (j = 0; (c1 = s1[j]) && (c2 = s2[j]); j++)
        {
            if (c1 != c2)
            {
                break;
            }
        }

        if (lcd == 0)
        {
            lcd = j;
        }
        else
        {
            if (lcd > j)
            {
                lcd = j;
            }
        }
    }

    return lcd;
}


static void cmd_free_runtime(CMD_ITEM_T *cmd_item)
{
    UINT32 i;
    UINT32 count;
    CMD_DESC_T *cmd_desc;

    count = UTIL_VectorMax(cmd_item->cmd_desc_vec);
    for (i = cmd_item->fixed_cmd_count; i < count; i++)
    {
        cmd_desc = UTIL_VectorSlot(cmd_item->cmd_desc_vec, i);
        if (cmd_desc)
        {
            if (cmd_desc->cmd)
            {
                VOS_FREE(cmd_desc->cmd);
            }

            if (cmd_desc->str)
            {
                VOS_FREE(cmd_desc->str);
            }

            VOS_FREE(cmd_desc);
            UTIL_VectorUnset(cmd_item->cmd_desc_vec, i);
        }
    }
}


static UBOOL8 cmd_match_element(UTIL_VECTOR line_vec,
                                CMD_ELEMENT_T *element,
                                int *argc,
                                char **argv)
{
    UINT32 i;
    UINT32 j;
    UINT32 count;
    UINT32 max_arg_count = *argc;
    CMD_ITEM_T *cmd_item;
    CMD_DESC_T *cmd_desc;
    char *cmd;

    *argc = 0;

    count = UTIL_VectorMax(line_vec);
    cmd = UTIL_VectorSlot(line_vec, count - 1);

    if (count > UTIL_VectorMax(element->strvec))
    {
        if ((count - 1) == UTIL_VectorMax(element->strvec))
        {
            if (cmd)
            {
                return FALSE;
            }
            else
            {
                count--;
            }
        }
        else
        {
            return FALSE;
        }
    }

    for (i = 0; i < count; i++)
    {
        cmd = UTIL_VectorSlot(line_vec, i);
        argv[(*argc)++] = cmd;

        cmd_item = UTIL_VectorSlot(element->strvec, i);
        cmd_free_runtime(cmd_item);

        for (j = 0; j < cmd_item->fixed_cmd_count; j++)
        {
            cmd_desc = UTIL_VectorSlot(cmd_item->cmd_desc_vec, j);
            if (CMD_RUNTIME(cmd_desc->cmd))
            {
                if (element->runtime_func)
                {
                    (*element->runtime_func)(*argc, argv, cmd_item->cmd_desc_vec);
                }
                break;
            }
        }

        (*argc)--;

        for (j = 0; j < UTIL_VectorMax(cmd_item->cmd_desc_vec); j++)
        {
            cmd_desc = UTIL_VectorSlot(cmd_item->cmd_desc_vec, j);
            if (cmd_desc && ! CMD_RUNTIME(cmd_desc->cmd))
            {
                if (cmd)
                {
                    if (CMD_VARIABLE(cmd_desc->cmd)
                     || (CMD_IPV4(cmd_desc->cmd) && cmd_ipv4_match(cmd))
                     || (CMD_IPV4_PREFIX(cmd_desc->cmd) && cmd_ipv4_prefix_match(cmd))
                     || (CMD_RANGE(cmd_desc->cmd) && cmd_range_match(cmd_desc->cmd, cmd)))
                    {
                        if (*argc < max_arg_count)
                        {
                            argv[(*argc)++] = cmd;
                        }
                        else
                        {
                            fprintf(stderr, "Arg count exceed %u\r\n", max_arg_count);
                        }
                        break;
                    }
                    else if (0 == strncasecmp(cmd, cmd_desc->cmd, strlen(cmd)))
                    {
                        if (UTIL_VectorMax(cmd_item->cmd_desc_vec) > 1)
                        {
                            if (*argc < max_arg_count)
                            {
                                argv[(*argc)++] = cmd;
                            }
                            else
                            {
                                fprintf(stderr, "Arg count exceed %u\r\n", max_arg_count);
                            }
                        }
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }

        if (cmd && j == UTIL_VectorMax(cmd_item->cmd_desc_vec))
        {
            return FALSE;
        }
    }

    return TRUE;
}


static void cmd_match(VTY_T *vty,
                      UTIL_VECTOR line_vec,
                      UTIL_VECTOR cmd_vec,
                      UBOOL8 hide,
                      int *argc,
                      char **argv)
{
    UINT32 i;
    CMD_ELEMENT_T *element;
    int j;
    int argc_temp;
    char *argv_temp[CMD_MAX_ARG_COUNT];

    for (i = 0; i < UTIL_VectorMax(cmd_vec); i++)
    {
        element = UTIL_VectorSlot(cmd_vec, i);

        if (NULL == element || ( ! hide && element->hide) || element->level > vty->privilege)
        {
            UTIL_VectorSetIndex(cmd_vec, i, NULL);
            continue;
        }

        if (cli_handleSelf == element->power_func)
        {
            if ( ! cmd_check_element_node(element, vty))
            {
                UTIL_VectorSetIndex(cmd_vec, i, NULL);
                continue;
            }
        }

        argc_temp = CMD_MAX_ARG_COUNT;
        memset(argv_temp, 0, sizeof(argv_temp));
        if (cmd_match_element(line_vec, element, &argc_temp, argv_temp))
        {
            if (UTIL_VectorMax(line_vec) == UTIL_VectorMax(element->strvec))
            {
                if (argc_temp < *argc)
                {
                    *argc = argc_temp;
                }

                for (j = 0; j < *argc; j++)
                {
                    argv[j] = argv_temp[j];
                }
            }
        }
        else
        {
            UTIL_VectorSetIndex(cmd_vec, i, NULL);
        }
    }
}


/* '?' describe command support. */
void cmd_describe_command(VTY_T *vty,
                          UTIL_VECTOR line_vec,
                          UTIL_VECTOR match_vec)
{
    UINT32 i;
    UINT32 j;
    UTIL_VECTOR cmd_vec;
    CMD_ELEMENT_T *element;
    CMD_ITEM_T *cmd_item;
    CMD_DESC_T *cmd_desc;
    int argc = CMD_MAX_ARG_COUNT;
    char *argv[CMD_MAX_ARG_COUNT];
    char *end_cmd = NULL;
    static CMD_DESC_T cmd_desc_cr = {"<cr>", "Please press ENTER to execute command"};

    cmd_vec = UTIL_VectorCopy(cmd_node_vector(vty->node));
    cmd_match(vty, line_vec, cmd_vec, FALSE, &argc, argv);

    i = UTIL_VectorMax(line_vec);
    if (i > 0)
    {
        end_cmd = UTIL_VectorSlot(line_vec, i - 1); 
    }

    for (i = 0; i < UTIL_VectorMax(cmd_vec); i++)
    {
        element = UTIL_VectorSlot(cmd_vec, i);
        if (element)
        {
            if (UTIL_VectorMax(line_vec) > UTIL_VectorMax(element->strvec))
            {
                if ( ! cmd_desc_unique_string(match_vec, cmd_desc_cr.cmd))
                {
                    UTIL_VectorSet(match_vec, &cmd_desc_cr);
                }
                continue;
            }

            cmd_item = UTIL_VectorSlot(element->strvec, UTIL_VectorMax(line_vec) - 1);

            for (j = 0; j < UTIL_VectorMax(cmd_item->cmd_desc_vec); j++)
            {
                cmd_desc = UTIL_VectorSlot(cmd_item->cmd_desc_vec, j);
                if (cmd_desc && ! CMD_RUNTIME(cmd_desc->cmd))
                {
                    if ( ! cmd_desc_unique_string(match_vec, cmd_desc->cmd))
                    {
                        if (NULL == end_cmd || 0 == util_strncasecmp(cmd_desc->cmd, end_cmd, util_strlen(end_cmd)))
                        {
                            UTIL_VectorSet(match_vec, cmd_desc);
                        }
                    }
                }
            }
        }
    }

    UTIL_VectorFree(cmd_vec);
}


/* Command line completion support. */
UINT32 cmd_complete_command(VTY_T *vty,
                            UTIL_VECTOR line_vec,
                            char match[][CMD_MAX_WORD_LEN],
                            UINT32 max_match_count,
                            UINT32 *match_len)
{
    UINT32 i;
    UINT32 j;
    UINT32 k;
    UINT32 lcd;
    UINT32 match_count;
    UTIL_VECTOR cmd_vec;
    CMD_ELEMENT_T *element;
    CMD_ITEM_T *cmd_item;
    CMD_DESC_T *cmd_desc;
    char *cmd;
    int argc = CMD_MAX_ARG_COUNT;
    char *argv[CMD_MAX_ARG_COUNT];

    cmd_vec = UTIL_VectorCopy(cmd_node_vector(vty->node));
    cmd_match(vty, line_vec, cmd_vec, FALSE, &argc, argv);

    match_count = 0;

    for (i = 0; i < UTIL_VectorMax(cmd_vec); i++)
    {
        element = UTIL_VectorSlot(cmd_vec, i);
        if (NULL == element)
        {
            continue;
        }

        if (UTIL_VectorMax(line_vec) > UTIL_VectorMax(element->strvec))
        {
            continue;
        }

        cmd = UTIL_VectorSlot(line_vec, UTIL_VectorMax(line_vec) - 1);
        cmd_item = UTIL_VectorSlot(element->strvec, UTIL_VectorMax(line_vec) - 1);

        for (j = 0; j < UTIL_VectorMax(cmd_item->cmd_desc_vec); j++)
        {
            cmd_desc = UTIL_VectorSlot(cmd_item->cmd_desc_vec, j);
            if (cmd_desc)
            {
                if (cmd_entry_function(cmd, cmd_desc->cmd))
                {
                    for (k = 0; k < match_count; k++)
                    {
                        if (0 == strcasecmp(match[k], cmd_desc->cmd))
                        {
                            break;
                        }
                    }

                    if (k == match_count)
                    {
                        if (match_count < max_match_count)
                        {
                            strncpy(match[match_count], cmd_desc->cmd, CMD_MAX_WORD_LEN -1);
                            match[match_count][CMD_MAX_WORD_LEN -1] = '\0';
                            match_count++;
                        }
                        else
                        {
                            fprintf(stderr, "Match count exceed %u\r\n", max_match_count);
                            break;
                        }
                    }
                }
            }
        }
    }

    UTIL_VectorFree(cmd_vec);

    *match_len = 0;

    if (match_count > 1)
    {
        cmd = UTIL_VectorSlot(line_vec, UTIL_VectorMax(line_vec) - 1);
        if (cmd)
        {
            lcd = cmd_lcd(match, match_count);
            if (lcd)
            {
                if (strlen(cmd) < lcd)
                {
                    *match_len = lcd;
                    return 0;
                }
            }
        }
    }

    return match_count;
}


/* Execute command by argument vline UTIL_VECTOR. */
UINT32 cmd_execute_command(VTY_T *vty, UTIL_VECTOR line_vec)
{
    UINT32 i;
    UTIL_VECTOR cmd_vec;
    CMD_ELEMENT_T *element;
    CMD_ELEMENT_T *match_element;
    UINT32 match_count;
    UINT32 incomplete_count;
    int argc = CMD_MAX_ARG_COUNT;
    char *argv[CMD_MAX_ARG_COUNT];

    cmd_vec = UTIL_VectorCopy(cmd_node_vector(vty->node));
    cmd_match(vty, line_vec, cmd_vec, TRUE, &argc, argv);

    match_count = 0;
    incomplete_count = 0;
    match_element = NULL;

    for (i = 0; i < UTIL_VectorMax(cmd_vec); i++)
    {
        element = UTIL_VectorSlot(cmd_vec, i);
        if (element)
        {
            if (UTIL_VectorMax(line_vec) == UTIL_VectorMax(element->strvec))
            {
                match_element = element;
                match_count++;
            }
            else
            {
                incomplete_count++;
            }
        }
    }

    UTIL_VectorFree(cmd_vec);

    if (match_count == 0)
    {
        if (incomplete_count > 0)
        {
            return CMD_ERR_INCOMPLETE;
        }
        else
        {
            return CMD_ERR_NO_MATCH;
        }
    }
    else if (match_count > 1)
    {
        return CMD_ERR_AMBIGUOUS;
    }

    if (match_element->power_func)
    {
        (*match_element->power_func)(match_element, vty, argc, argv);
    }

    if (match_element->func)
    {
        (*match_element->func)(vty, argc, argv);
    }
        
    return CMD_SUCCESS;
}


char *cmd_prompt(CLI_NODE_ID nodeid)
{
    CMD_NODE_T *node;

    node = UTIL_VectorSlot(sg_cmdvec, nodeid);
    return node->prompt;
}


CLI_NODE_ID cmd_find_node(CLI_NODE_ID rootNodeId, const char *path)
{
    char *ptr;
    int i;
    CLI_NODE_ID nodeId = INVALID_NODE;
    CMD_NODE_T *node = NULL;
    UTIL_VECTOR cmdVec;
    CMD_ELEMENT_T *element;
    char prompt[CMD_PROMPT_SIZE];

    node = UTIL_VectorSlot(sg_cmdvec, rootNodeId);
    cmdVec = node->cmd_vector;

    ptr = strchr(path, '/');
    if (NULL != ptr)
    {
        strncpy(prompt, path, CMD_PROMPT_SIZE);
        *(prompt + (ptr - path)) = '\0';
    }
    for (i = 0; i < UTIL_VectorMax(cmdVec); i++)
    {
        element = UTIL_VectorSlot(cmdVec, i);
        if (NULL == ptr && cli_handleSelf == element->power_func && 0 == strcasecmp(path, element->string))
        {
            return element->node;
        }
        else if (NULL != ptr && cli_handleSelf == element->power_func && 0 == strcasecmp(prompt, element->string))
        {
            path = ptr + 1;
            nodeId = cmd_find_node(element->node, path);
        }
        else
        {
            continue;
        }
    }
    return nodeId;
}


void cmd_install_node(CMD_NODE_T *node, CLI_FUNC func)
{
    UTIL_VectorSetIndex(sg_cmdvec, node->node, node);

    node->func = func;
    node->cmd_vector = UTIL_VectorInit(CMD_VECTOR_DEFAULT_SIZE);
}


void cmd_install_element(CLI_NODE_ID nodeid, CMD_ELEMENT_T *element)
{
    CMD_NODE_T *node;

    node = UTIL_VectorSlot(sg_cmdvec, nodeid);

    if (node == NULL)
    {
        fprintf(stderr, "Command node %u doesn't exist, please check it\r\n", nodeid);
        exit(1);
    }

    UTIL_VectorSet(node->cmd_vector, element);

    element->strvec = cmd_make_descvec(element->string, element->doc);
}


void cmd_install_cmddesc(const char *cmd, const char *desc, UTIL_VECTOR cmd_desc_vec)
{
    CMD_DESC_T *cmd_desc;

    cmd_desc = VOS_MALLOC(sizeof(CMD_DESC_T));
    if (NULL == cmd_desc)
    {
        return;
    }

    memset(cmd_desc, 0, sizeof(CMD_DESC_T));

    cmd_desc->cmd = VOS_STRDUP(cmd);
    if (NULL == cmd_desc->cmd)
    {
        VOS_FREE(cmd_desc);
        return;
    }

    cmd_desc->str = VOS_STRDUP(desc);
    if (NULL == cmd_desc->str)
    {
        VOS_FREE(cmd_desc->cmd);
        VOS_FREE(cmd_desc);
        return;
    }

    UTIL_VectorSet(cmd_desc_vec, cmd_desc);
}


/* Breaking up string into each command piece. I assume given
   character is separated by a space character. Return value is a
   UTIL_VECTOR which includes char ** data element. */
UTIL_VECTOR cmd_make_strvec(char *cmd)
{
    UINT32 len;
    char *cp;
    char *start;
    char *token;
    UTIL_VECTOR strvec;

    if (cmd == NULL)
    {
        return NULL;
    }

    cp = cmd;

    while (isspace((int) *cp) &&  *cp != '\0')
    {
        cp++;
    }

    if (*cp == '\0')
    {
        return NULL;
    }

    if (*cp == '!' ||  *cp == '#')
    {
        return NULL;
    }

    strvec = UTIL_VectorInit(CMD_VECTOR_DEFAULT_SIZE);

    /* Copy each cmd piece and set into UTIL_VECTOR. */
    while (1)
    {
        start = cp;
        if ('\"' == *cp)
        {
            cp++;
            while ('\"' != *cp &&  *cp != '\0')
            {
                cp++;
            }

            if ('\"' == *cp)
            {
                cp++;
            }
        }
        else
        {
            while ( ! isspace((int) *cp) &&  *cp != '\0')
            {
                cp++;
            }
        }

        len = cp - start;
        token = VOS_MALLOC(len + 1);
        memcpy(token, start, len);
        *(token + len) = '\0';
        UTIL_VectorSet(strvec, token);

        while (isspace((int) *cp) &&  *cp != '\0')
        {
            cp++;
        }

        if (*cp == '\0')
        {
            return strvec;
        }
    }
}


void cmd_free_strvec(UTIL_VECTOR strvec)
{
    UINT32 i;
    char *cp;

    if (!strvec)
    {
        return ;
    }

    for (i = 0; i < UTIL_VectorMax(strvec); i++)
    {
        cp = UTIL_VectorSlot(strvec, i);
        if (cp != NULL)
        {
            VOS_FREE(cp);
        }
    }

    UTIL_VectorFree(strvec);
}


/* Initialize command interface. Install basic nodes and commands. */
VOS_RET_E cmd_init()
{
    /* Allocate initial top UTIL_VECTOR of commands. */
    sg_cmdvec = UTIL_VectorInit(CMD_VECTOR_DEFAULT_SIZE);
    if (sg_cmdvec)
    {
        return VOS_RET_SUCCESS;
    }
    else
    {
        return VOS_RET_RESOURCE_EXCEEDED;
    }
}
