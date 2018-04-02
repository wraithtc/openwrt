#include "fwk.h"
#include "cli_vty.h"
#include "cli_cmd.h"
#include "cli_buf.h"
#include "cli_init.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <termios.h>
#include <sys/ioctl.h> 


/* Max command that user can input */
#define VTY_MAX_CMD_LEN    1124

#define VTY_READ_BUFSIZE 512
#define VTY_DEFAULT_WIDTH  140
#define VTY_BUFSIZE   1200

#define VTY_PRE_ESCAPE   1
#define VTY_ESCAPE   2
#define VTY_DELESC   3
#define VTY_INSERTESC   4
#define VTY_HOMEESC   5
#define VTY_ENDESC   6

#define VTY_COMPLETE_MAX_COUNT   500

#define VTY_BACKWARD   "\x08"
#define VTY_SPACE   " "

#define VTY_CONTROL(X)  ((X) - '@')
#define VTY_NEWLINE  ((vty->type == VTY_TERM) ? "\r\n" : "\r\n")


static void vty_welcome(void)
{
    return ;
}


static void vty_write(VTY_T *vty, char *buf, UINT32 nbytes)
{
    buffer_write(vty->obuf, (unsigned char *)buf, nbytes);
}


static void vty_prompt(VTY_T *vty)
{
    if ((vty->type == VTY_TERM) && (vty->status != VTY_CLOSE))
    {
        printf(cmd_prompt(vty->node), CLI_PROMPT_HOST);
        if (vty->lazy_buf)
        {
            printf("{%s}", vty->lazy_buf);
        }

        if (CLI_LEVEL_GUEST == vty->privilege)
        {
            printf("%s", CLI_PROMPT_GUEST_END);
        }
        else
        {
            printf("%s", CLI_PROMPT_END);
        }

        fflush(stdout);
    }

    return ;
}


static void vty_redraw_line(VTY_T *vty)
{
    vty_write(vty, vty->buf, vty->length);
    vty->cp = vty->length;
}


static int vty_ensure(VTY_T *vty, int length)
{
    if (vty->max <= length)
    {
        char *pr = NULL;
        pr = (char *)UTIL_VectorRealloc(vty->buf, vty->max, (vty->max) * 2);
        if (pr == NULL)
        {
            return  - 1;
        }

        vty->buf = pr;
        vty->max *= 2;
    }
    return 1;
}


static void vty_clear_buf(VTY_T *vty)
{
    memset((char *)vty->buf, 0, vty->max);
}


/* Basic function to insert character into vty. */
static void vty_self_insert(VTY_T *vty, char c)
{
    int i;
    int length;

    if (vty->length >= VTY_MAX_CMD_LEN)
    {
        printf("%s%% Too long input command.%s", VTY_NEWLINE, VTY_NEWLINE);

        /* Clear command line buffer. */
        vty->cp = vty->length = 0;
        vty_clear_buf(vty);
        if (vty->status != VTY_CLOSE && vty->status != VTY_START && vty->status != VTY_CONTINUE)
        {
            vty_prompt(vty);
        }

        return ;
    }

    if (vty_ensure(vty, vty->length + 1) < 0)    /* added by ouzhigang 2002/05/17 */
    {
        return ;
    }

    length = vty->length - vty->cp;
    memmove(&vty->buf[vty->cp + 1], &vty->buf[vty->cp], length);
    vty->buf[vty->cp] = c;
    vty_write(vty, &vty->buf[vty->cp], length + 1);
    for (i = 0; i < length; i++)
    {
        vty_write(vty, VTY_BACKWARD, 1);
    }
    vty->cp++;
    vty->length++;
}


/* Insert a word into vty interface with overwrite mode. */
static void vty_insert_word_overwrite(VTY_T *vty, char *str)
{
    int len = strlen(str);

    vty_write(vty, str, len);
    UTIL_STRNCPY(&vty->buf[vty->cp], str, vty->max - vty->cp);
    vty->cp += len;
    vty->length = vty->cp;
}


/* Forward character. */
static void vty_forward_char(VTY_T *vty)
{
    if (vty->cp < vty->length)
    {
        vty_write(vty, &vty->buf[vty->cp], 1);
        vty->cp++;
    }
}


/* Backward character. */
static void vty_backward_char(VTY_T *vty)
{
    if (vty->cp > 0)
    {
        vty->cp--;
        vty_write(vty, VTY_BACKWARD, 1);
    }
}


/* Move to the beginning of the line. */
static void vty_beginning_of_line(VTY_T *vty)
{
    while (vty->cp)
    {
        vty_backward_char(vty);
    }
}


#if 0
/* Move to the end of the line. */
static void vty_end_of_line(VTY_T *vty)
{
    while (vty->cp < vty->length)
    {
        vty_forward_char(vty);
    }
}
#endif


/* Forward word. */
static void vty_forward_word(VTY_T *vty)
{
    while (vty->cp != vty->length && vty->buf[vty->cp] != ' ')
    {
        vty_forward_char(vty);
    }
    while (vty->cp != vty->length && vty->buf[vty->cp] == ' ')
    {
        vty_forward_char(vty);
    }
}


/* Backward word without skipping training space. */
static void vty_backward_pure_word(VTY_T *vty)
{
    while (vty->cp > 0 && vty->buf[vty->cp - 1] != ' ')
    {
        vty_backward_char(vty);
    }
}


/* Backward word. */
static void vty_backward_word(VTY_T *vty)
{
    while (vty->cp > 0 && vty->buf[vty->cp - 1] == ' ')
    {
        vty_backward_char(vty);
    }
    while (vty->cp > 0 && vty->buf[vty->cp - 1] != ' ')
    {
        vty_backward_char(vty);
    }
}


static void vty_delete_char(VTY_T *vty)
{
    int i;
    int size;

    if (vty->cp == vty->length)
    {
        return ; /* completion need here? */
    }

    size = vty->length - vty->cp;
    vty->length--;
    memmove(&vty->buf[vty->cp], &vty->buf[vty->cp + 1], size - 1);
    vty->buf[vty->length] = '\0';
    vty_write(vty, &vty->buf[vty->cp], size - 1);
    vty_write(vty, VTY_SPACE, 1);
    for (i = 0; i < size; i++)
    {
        vty_write(vty, VTY_BACKWARD, 1);
    }
}


/* Delete a character before the point. */
static void vty_delete_backward_char(VTY_T *vty)
{
    if (vty->cp == 0)
    {
        return ;
    }
    vty_backward_char(vty);
    vty_delete_char(vty);
}


static void vty_kill_line(VTY_T *vty)
{
    int i;
    int size;

    size = vty->length - vty->cp;
    if (size == 0)
    {
        return ;
    }
    for (i = 0; i < size; i++)
    {
        vty_write(vty, VTY_SPACE, 1);
    }
    for (i = 0; i < size; i++)
    {
        vty_write(vty, VTY_BACKWARD, 1);
    }
    memset((char*) &vty->buf[vty->cp], 0, size);
    vty->length = vty->cp;
}


static void vty_kill_line_from_beginning(VTY_T *vty)
{
    vty_beginning_of_line(vty);
    vty_kill_line(vty);
}


/* Delete a word before the point. */
static void vty_forward_kill_word(VTY_T *vty)
{
    while (vty->cp != vty->length && vty->buf[vty->cp] == ' ')
    {
        vty_delete_char(vty);
    }
    while (vty->cp != vty->length && vty->buf[vty->cp] != ' ')
    {
        vty_delete_char(vty);
    }
}


/* Delete a word before the point. */
static void vty_backward_kill_word(VTY_T *vty)
{
    while (vty->cp > 0 && vty->buf[vty->cp - 1] == ' ')
    {
        vty_delete_backward_char(vty);
    }
    while (vty->cp > 0 && vty->buf[vty->cp - 1] != ' ')
    {
        vty_delete_backward_char(vty);
    }
}


/* Add current command line to the history buffer. */
static void vty_hist_add(VTY_T *vty)
{
    int idx;

    if (vty->length == 0)
    {
        return ;
    }
    idx = vty->hindex ? vty->hindex - 1: VTY_MAXHIST - 1;

    /* Ignore the same string as previous one. */
    if (vty->hist[idx])
    {
        if (strcmp(vty->buf, vty->hist[idx]) == 0)
        {
            vty->hp = vty->hindex;
            return ;
        }
    }

    /* Insert history entry. */
    if (vty->hist[vty->hindex])
    {
        VOS_FREE(vty->hist[vty->hindex]);
    }
    vty->hist[vty->hindex] = VOS_STRDUP(vty->buf);
    if (vty->hist[vty->hindex] == NULL)    /* add by liuyanjun 2002/08/15 */
    {
        UTIL_Assert(0);
        return ;
    } /* end */

    /* History idx rotation. */
    vty->hindex++;
    if (vty->hindex == VTY_MAXHIST)
    {
        vty->hindex = 0;
    }
    vty->hp = vty->hindex;
}


/* Print command line history.  This function is called from
   vty_next_line and vty_previous_line. */
static void vty_history_print(VTY_T *vty)
{
    int length;

    vty_kill_line_from_beginning(vty);

    /* Get previous line from history buffer */
    length = strlen(vty->hist[vty->hp]);
    memcpy(vty->buf, vty->hist[vty->hp], length);
    vty->cp = vty->length = length;

    /* Redraw current line */
    vty_redraw_line(vty);
}


/* Show next command line history. */
static void vty_next_line(VTY_T *vty)
{
    int try_index;

    if (vty->hp == vty->hindex)
    {
        return ;
    }

    /* Try is there history exist or not. */
    try_index = vty->hp;
    if (try_index == (VTY_MAXHIST - 1))
    {
        try_index = 0;
    }
    else
    {
        try_index++;
    }

    /* If there is not history return. */
    if (vty->hist[try_index] == NULL)
    {
        return ;
    }
    else
    {
        vty->hp = try_index;
    }

    vty_history_print(vty);
}


/* Show previous command line history. */
static void vty_previous_line(VTY_T *vty)
{
    int try_index;

    try_index = vty->hp;
    if (try_index == 0)
    {
        try_index = VTY_MAXHIST - 1;
    }
    else
    {
        try_index--;
    }
    if (vty->hist[try_index] == NULL)
    {
        return ;
    }
    else
    {
        vty->hp = try_index;
    }
    vty_history_print(vty);
}


static char * vty_make_input_buf(VTY_T *vty)
{
    int len;
    char *buf;
    char *trim_buf;

    if (vty->lazy_buf)
    {
        trim_buf = VOS_STRDUP(vty->buf);
        util_strTrimL(trim_buf);

        if ('.' == trim_buf[0] && '.' == trim_buf[1])
        {
            len = strlen(vty->lazy_buf);
            for (buf = vty->lazy_buf + len - 1; buf >= vty->lazy_buf; buf--)
            {
                if (isspace(*buf))
                {
                    break;
                }
            }

            if (buf < vty->lazy_buf)
            {
                VOS_FREE(vty->lazy_buf);
                vty->lazy_buf = NULL;
            }
            else
            {
                *buf = '\0';
            }

            VOS_FREE(trim_buf);            
            return NULL;
        }
        else if ('/' == trim_buf[0])
        {
            VOS_FREE(vty->lazy_buf);
            vty->lazy_buf = NULL;

            VOS_FREE(trim_buf);            
            return NULL;
        }

        VOS_FREE(trim_buf);            
    }

    len = strlen(vty->buf) + 2;
    if (vty->lazy_buf)
    {
        len += strlen(vty->lazy_buf);
    }

    buf = VOS_MALLOC(len);
    if (NULL == buf)
    {
        return NULL;
    }

    memset(buf, 0, len);

    if (vty->lazy_buf)
    {
        UTIL_STRNCPY(buf, vty->lazy_buf, len);
        UTIL_STRNCAT(buf, " ", len);
    }

    UTIL_STRNCAT(buf, vty->buf, len);

    return buf;
}


/* Do completion at vty interface. */
static void vty_complete_command(VTY_T *vty)
{
    UINT32 i;
    UINT32 matchcount;
    UINT32 matchlen;
    char *input_buf;
    char word[CMD_MAX_WORD_LEN];
    char matched[VTY_COMPLETE_MAX_COUNT][CMD_MAX_WORD_LEN];
    UTIL_VECTOR vline;

    input_buf = vty_make_input_buf(vty);
    if (input_buf == NULL)
    {
        return ;
    }

    vline = cmd_make_strvec(input_buf);
    if (vline == NULL)
    {
        VOS_FREE(input_buf);
        return ;
    }

    /* In case of 'help \t'. */
    if (isspace((int)input_buf[strlen(input_buf) - 1]))
    {
        UTIL_VectorSet(vline, NULL);
    }
    printf("%s", VTY_NEWLINE);

    matchlen = 1;
    matchcount = cmd_complete_command(vty, vline, matched, VTY_COMPLETE_MAX_COUNT, &matchlen);

    cmd_free_strvec(vline);

    if (0 == matchcount)
    {
        if (matchlen > 0)
        {
            strncpy(word, matched[0], matchlen);
            word[matchlen] = '\0';

            vty_prompt(vty);
            vty_redraw_line(vty);
            vty_backward_pure_word(vty);
            vty_insert_word_overwrite(vty, word);
        }
        else
        {
            printf("%% There is no matched command.%s", VTY_NEWLINE);
            vty_prompt(vty);
            vty_redraw_line(vty);
        }
    }
    else if (1 == matchcount)
    {
        vty_prompt(vty);
        vty_redraw_line(vty);
        vty_backward_pure_word(vty);
        vty_insert_word_overwrite(vty, matched[0]);
        vty_self_insert(vty, ' ');
    }
    else
    {
        UINT32 max_len;
        UINT32 len;
        UINT32 max_count;

        max_len = 0;
        for (i = 0; i < matchcount; i++)
        {
            len = util_strlen(matched[i]);
            if (len > max_len)
            {
                max_len = len;
            }
        }

        max_count = vty_get_win_width() / max_len;
        if (0 == max_count)
        {
            max_count = 1;
        }

        for (i = 0; i < matchcount; i++)
        {
            if (i != 0 && ((i % max_count) == 0))
            {
                printf("%s", VTY_NEWLINE);
            }
            printf("%-*s ", max_len, matched[i]);
        }
        printf("%s", VTY_NEWLINE);

        vty_prompt(vty);
        vty_redraw_line(vty);
    }

    VOS_FREE(input_buf);

    return ;
}


static void vty_describe_fold(VTY_T *vty, int cmdwidth, UINT32 descwidth, CMD_DESC_T *cmddesc)
{
    char buf[VTY_DEFAULT_WIDTH];
    char *cmd;
    char *p;
    UINT32 pos;

    cmd = cmddesc->cmd;
    if (descwidth <= 0)
    {
        printf("  %-*s  \t\t\t%s%s", cmdwidth, cmd, cmddesc->str, VTY_NEWLINE);
        return ;
    }

    for (p = cmddesc->str; strlen(p) > descwidth; p += pos + 1)
    {
        for (pos = descwidth; pos > 0; pos--)
        {
            if (*(p + pos) == ' ')
            {
                break;
            }
        }

        if (pos == 0)
        {
            break;
        }

        strncpy(buf, p, pos);
        buf[pos] = '\0';
        printf("  %-*s \t\t\t%s%s", cmdwidth, cmd, buf, VTY_NEWLINE);
        cmd = "";
    }

    printf("  %-*s \t\t\t%s%s", cmdwidth, cmd, p, VTY_NEWLINE);
}


/* Describe matched command function. */
static void vty_describe_command(VTY_T *vty)
{
    UINT32 i;
    UINT32 len;
    UINT32 width;
    UINT32 descwidth;
    char *input_buf;
    UTIL_VECTOR vline;
    UTIL_VECTOR describe = NULL;
    CMD_DESC_T *desc;

    input_buf = vty_make_input_buf(vty);
    if (input_buf == NULL)
    {
        return;
    }

    vline = cmd_make_strvec(input_buf); /*input_buf has't '?'*/

    /* In case of '> ?'. */
    if (vline == NULL)
    {
        vline = UTIL_VectorInit(1);
        if (vline == NULL)
        {
            VOS_FREE(input_buf);
            return ;
        }
        UTIL_VectorSet(vline, NULL);
    }
    else if (isspace((int)input_buf[strlen(input_buf) - 1]))    /*there is input "space+?"*/
    {
        UTIL_VectorSet(vline, NULL);
    }
    printf("%s", VTY_NEWLINE);

    describe = UTIL_VectorInit(CMD_INIT_MATCHVEC_SIZE);
    if (NULL == describe)
    {
        VOS_FREE(input_buf);
        cmd_free_strvec(vline);
        return ;
    }

    cmd_describe_command(vty, vline, describe);

    desc = UTIL_VectorSlot(describe, 0);
    if (NULL == desc)
    {
        printf("%% There is no matched command.%s", VTY_NEWLINE);
        VOS_FREE(input_buf);
        cmd_free_strvec(vline);
        vty_prompt(vty);
        vty_redraw_line(vty);
        return ;
    }

    /* Get width of command string. */
    width = 0;
    for (i = 0; i < UTIL_VectorMax(describe); i++)
    {
        desc = UTIL_VectorSlot(describe, i);
        if (desc != NULL)
        {
            len = strlen(desc->cmd);
            if (len > 0)
            {
                if (width < len)
                {
                    width = len;
                }
            }
        }
    }

    /* Get width of description string. */
    if (vty->width != 0)
    {
        descwidth = vty->width - (width + 6);
    }
    else
    {
        descwidth = VTY_DEFAULT_WIDTH - (width + 6);
    }

    /* Print out description. */
    for (i = 0; i < UTIL_VectorMax(describe); i++)
    {
        desc = UTIL_VectorSlot(describe, i);
        if (desc != NULL)
        {
            if (desc->cmd[0] == '\0')
            {
                continue;
            }

            if (!desc->str)
            {
                printf("  %-s%s", desc->cmd, VTY_NEWLINE);
            }
            else if (descwidth >= strlen(desc->str))
            {
                printf("  %-*s \t\t\t%s%s", width, desc->cmd, desc->str, VTY_NEWLINE);
            }
            else
            {
                vty_describe_fold(vty, width, descwidth, desc);
            }
        }
    }

    UTIL_VectorFree(describe);
    VOS_FREE(input_buf);
    cmd_free_strvec(vline);
    vty_prompt(vty);
    vty_redraw_line(vty);

    return;
}


/* Command execution over the vty interface. */
int vty_command(VTY_T *vty)
{
    int ret;
    char *input_buf;
    UTIL_VECTOR vline;

    input_buf = vty_make_input_buf(vty);
    if (input_buf == NULL)
    {
        return CMD_ERR_NO_MATCH;
    }

    vline = cmd_make_strvec(input_buf);
    if (vline == NULL)
    {
        VOS_FREE(input_buf);
        return CMD_ERR_NO_MATCH;
    }

    ret = cmd_execute_command(vty, vline);
    switch (ret)
    {
        case CMD_SUCCESS:
            break;

        case CMD_ERR_EXEED_ARGC_MAX:
            printf("%% Too more argv.%s", VTY_NEWLINE);
            break;

        case CMD_ERR_AMBIGUOUS:
            printf("%% Ambiguous command.%s", VTY_NEWLINE);
            break;

        case CMD_ERR_NO_MATCH:
            printf("%% Unknown command.%s", VTY_NEWLINE);
            break;

        case CMD_ERR_INCOMPLETE:
            VOS_FREE(vty->lazy_buf);
            util_strTrim(input_buf);
            vty->lazy_buf = input_buf;
            cmd_free_strvec(vline);
            printf("%% Enter lazy mode, input '..' to back, input '/' to quit.%s", VTY_NEWLINE);
            return CMD_SUCCESS;

        default:
            printf("%% Unknow error %d.%s", ret, VTY_NEWLINE);
            break;
    }

    cmd_free_strvec(vline);
    VOS_FREE(input_buf);

    return ret;
}


/* Execute current command line. */
static int vty_execute(VTY_T *vty)
{
    int ret;

    ret = CMD_SUCCESS;

    switch (vty->node)
    {
        default:
            ret = vty_command(vty);
            if (vty->type == VTY_TERM)
            {
                vty_hist_add(vty);
            }
            break;
    }

    /* Clear command line buffer. */
    vty->cp = vty->length = 0;
    vty_clear_buf(vty);

    if ((vty->status != VTY_CLOSE) && (vty->status != VTY_START) && (vty->status != VTY_CONTINUE))
    {
        vty_prompt(vty);
    }

    return ret;
}


/* Escape character command map. */
static void vty_escape_map(unsigned char c, VTY_T *vty)
{
    switch (c)
    {
        case ('A'):
            vty_previous_line(vty);
            break;

        case ('B'):
            vty_next_line(vty);
            break;

        case ('C'):
            vty_forward_char(vty);
            break;

        case ('D'):
            vty_backward_char(vty);
            break;

        default:
            break;
    }

    vty->escape = VTY_NORMAL;
}


int vty_read(VTY_T *vty, const char *cmdLine)
{
    int n = 0;
    int i = 0;
    char c;

    n = strlen(cmdLine);
    for (i = 0; i < n; i++)
    {
        c = cmdLine[i];

        /* Escape character. */
        if (vty->escape == VTY_ESCAPE)
        {
            vty_escape_map(c, vty);
            continue;
        }

        /* Pre-escape status. */
        if (vty->escape == VTY_PRE_ESCAPE)
        {
            switch (c)
            {
                case '[':
                    vty->escape = VTY_ESCAPE;
                    break;

                case 'b':
                    vty_backward_word(vty);
                    vty->escape = VTY_NORMAL;
                    break;

                case 'f':
                    vty_forward_word(vty);
                    vty->escape = VTY_NORMAL;
                    break;

                case 'd':
                    vty_forward_kill_word(vty);
                    vty->escape = VTY_NORMAL;
                    break;

                case VTY_CONTROL('H'):
                case 0x7f:
                    vty_backward_kill_word(vty);
                    vty->escape = VTY_NORMAL;
                    break;

                default:
                    vty->escape = VTY_NORMAL;
                    break;
            }
            continue;
        }

        switch (c)
        {
            case VTY_CONTROL('A'):
                vty_beginning_of_line(vty);
                break;

            case VTY_CONTROL('B'):
                vty_backward_char(vty);
                break;

            case VTY_CONTROL('C'): 
                //vty_stop_input ( vty );
                break;

            case VTY_CONTROL('D'): 
                //case 0x7f:
                vty_delete_char(vty);
                break;

            case VTY_CONTROL('F'):
                vty_forward_char(vty);
                break;

            case VTY_CONTROL('H'):
            case 0x7f:
                vty_delete_backward_char(vty);
                break;

            case VTY_CONTROL('K'):
                vty_kill_line(vty);
                break;

            case VTY_CONTROL('N'):
                vty_next_line(vty);
                break;

            case VTY_CONTROL('P'):
                vty_previous_line(vty);
                break;

            case VTY_CONTROL('T'): 
                //vty_transpose_chars ( vty );
                break;

            case VTY_CONTROL('U'):
                vty_kill_line_from_beginning(vty);
                break;

            case VTY_CONTROL('W'):
                vty_backward_kill_word(vty);
                break;

            case VTY_CONTROL('Z'): 
                //vty_end_config ( vty );
                break;

            case '\n':
                if (i > 0)
                {
                    if (cmdLine[i - 1] == '\r')
                    {
                        break;
                    }
                }
            case '\r':
                printf("%s", VTY_NEWLINE);
                vty_execute(vty);
                break;

            case '\t':
                vty_complete_command(vty);
                break;

            case '?':
                vty_describe_command(vty);
                break;

            case '\033':
                /* '[' is for unix and win2k like telnet client , 'O' is for win9x like telnet client */
                if (i + 1 < n && ((cmdLine[i + 1] == '[') || (cmdLine[i + 1] == 'O')))
                {
                    vty->escape = VTY_ESCAPE;
                    i++;
                }
                else
                {
                    vty->escape = VTY_PRE_ESCAPE;
                }
                break;

            default:
                if (c > 31 && c < 127)
                {
                    vty_self_insert(vty, c);
                }
                break;
        }
    }

    buffer_flush_window(vty->obuf, vty->width, 20, 0, 1);

    return 0;
}


void vty_init_term(void)
{
    struct termios term;

    tcgetattr(STDIN_FILENO, &term);

    term.c_lflag &= ~ECHO; /* Turn off local Echo */
    term.c_lflag &= ~ISIG; /* Turn off Signal generation */
    term.c_lflag &= ~ICANON; /* Turn off canonical mode */

    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}


void vty_exit_term(void)
{
    struct termios term;

    tcgetattr(STDIN_FILENO, &term);

    term.c_lflag |= ECHO; /* Turn on local Echo */
    term.c_lflag |= ISIG; /* Turn on Signal generation */
    term.c_lflag |= ICANON; /* Turn on canonical mode */

    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}


UINT32 vty_get_win_width(void)
{
    int ret;
    struct winsize size; 

    ret = ioctl(STDIN_FILENO, TIOCGWINSZ, &size);
    if (ret)
    {
        return 80;
    }

    return size.ws_col;
}



/* Allocate new vty struct. */
VTY_T *vty_create(int id)
{
    VTY_T *new_vty = NULL;

    vty_init_term();
    cmd_init();
    CLI_init();

    new_vty = (VTY_T*)VOS_MALLOC(sizeof(VTY_T));
    if (new_vty == NULL)
    {
        UTIL_Assert(0);
        return NULL;
    }
    memset((char*)new_vty, 0, sizeof(VTY_T));
    new_vty->obuf = (struct buffer*)buffer_new(BUFFER_VTY, 128);
    if (new_vty->obuf == NULL)
    {
        UTIL_Assert(0);
        VOS_FREE(new_vty);
        return NULL;
    } 

    new_vty->buf = (char*)VOS_MALLOC(VTY_BUFSIZE);
    if (new_vty->buf == NULL)
    {
        UTIL_Assert(0);
        buffer_free(new_vty->obuf);
        VOS_FREE(new_vty);
        return NULL;
    }

    new_vty->max = VTY_BUFSIZE;
    new_vty->type = VTY_TERM;
    new_vty->node = ROOT_NODE;
    new_vty->cp = 0;
    vty_clear_buf(new_vty);
    new_vty->length = 0;
    memset((char*)new_vty->hist, 0, sizeof(new_vty->hist));
    new_vty->hp = 0;
    new_vty->hindex = 0;
    new_vty->status = VTY_NORMAL;
    new_vty->escape = VTY_NORMAL;
    new_vty->config = 0;

    vty_welcome();
    vty_prompt(new_vty);

    return new_vty;
}


/* add by chen xiao long */
void vty_free(VTY_T *vty)
{
    int i;

    if (!vty)
    {
        return ;
    }

    vty_exit_term();

    if (vty->obuf)
    {
        buffer_free(vty->obuf);
    }
    if (vty->buf)
    {
        VOS_FREE(vty->buf);
    }
    if (vty->lazy_buf)
    {
        VOS_FREE(vty->lazy_buf);
    }

    for (i = 0; i < VTY_MAXHIST; i++)
    {
        if (vty->hist[i])
        {
            VOS_FREE(vty->hist[i]);
        }
    }
    if (vty->index)
    {
        VOS_FREE(vty->index);
    }
    if (vty->index_sub)
    {
        VOS_FREE(vty->index_sub);
    }

    VOS_FREE(vty);
    return ;
}
