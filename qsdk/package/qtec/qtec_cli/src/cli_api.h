#ifndef __CLI_API_H__
#define __CLI_API_H__

#include "fwk.h"
#include "cli_vty.h"



/** Max number of failed authentications before we insert a delay penalty on the user. */
#define CLI_AUTH_NUM_MAX    (3)

/** Max number of seconds to wait for the MDM lock */
#define CLI_LOCK_TIMEOUT    (6 * MSECS_IN_SEC)


struct cmd_element;
typedef VOS_RET_E (*CLI_POWER_FUNC)(struct cmd_element *, VTY_T *, int, char **);
typedef VOS_RET_E (*CLI_FUNC)(VTY_T *, int, char **);
typedef VOS_RET_E (*CLI_RUNTIME_FUNC)(int, char **, UTIL_VECTOR);


/*!\enum NetworkAccessMode
 * \brief Different types of network access modes, returned by cmcDal_getNetworkAccessMode
 *        and also in the wordData field of VOS_MSG_GET_NETWORK_ACCESS_MODE
 */
typedef enum
{
    NETWORK_ACCESS_DISABLED = 0,  /**< access denied */
    NETWORK_ACCESS_LAN_SIDE = 1,  /**< access from LAN */
    NETWORK_ACCESS_WAN_SIDE = 2,  /**< access from WAN */
    NETWORK_ACCESS_CONSOLE = 3 /**< access from serial console */
} NetworkAccessMode;


typedef struct
{
    char *cmd;
    char *help;
    CLI_FUNC func;
    CLI_RUNTIME_FUNC runtime_func;
    CLI_LEVEL level;
} CLI_CMD_T;


typedef struct cmd_node
{
    /* Node index. */
    CLI_NODE_ID node;

    /* Prompt character at vty interface. */
    char *prompt;

    /* Node's configuration write function */
    CLI_FUNC func;

    /* Vector of this node's command list. */
    UTIL_VECTOR cmd_vector;

} CMD_NODE_T;


/* Structure of command element. */
typedef struct cmd_element
{
    CLI_NODE_ID node;

    /* Command specification by string. */
    char *string;

    CLI_POWER_FUNC power_func;
    CLI_FUNC func;
    CLI_RUNTIME_FUNC runtime_func;

    /* Documentation of this command. */
    char *doc;

    UBOOL8 hide;

    /* privilege level */
    CLI_LEVEL level;

    /* Pointing out each description vector. */
    UTIL_VECTOR strvec;

    /* Configuration string */
    char *config;

    /* Sub configuration string */
    UTIL_VECTOR subconfig;
} CMD_ELEMENT_T;


/** Print a message identifying the modem.
 */
void cli_printWelcomeBanner(void);


/** Main entry point into the CLI library.
 *
 * @param msgHandle (IN) message handle that was initialized by the caller.
 *                       Must be given to the CLI library so it can send messages out.
 * @param exitOnIdleTimeout (IN) The amount of time, in seconds, of idle-ness before
 *                               timing out.
 */
void cli_run(VTY_T *vty, void *msgHandle, UINT32 exitOnIdleTimeout);


/** Get username and password from the user and authenticate.
 *  This function will keep trying to authenticate the user until success,
 *  exit-on-idle timeout, or signal interrupt.
 *
 * @param accessMode (IN) an enum describing where the access is coming from.
 * @param exitOnIdleTimeout (IN) The amount of time, in seconds, of idle-ness before
 *                               timing out.
 *
 * @return VOS_RET_SUCCESS if authentication was successful.
 *         VOS_RET_TIMED_OUT if user stops typing for the exit-on-idle number of seconds.
 *         VOS_RET_OP_INTR if input was interrupted by a signal.
 *
 */
VOS_RET_E cli_authenticate(NetworkAccessMode accessMode,
                           UINT32 exitOnIdleTimeout,
                           CLI_LEVEL *level,
                           CLI_NODE_ID *perm);


/** Use this to specify application data
 *
 * @param appName (IN) name of application
 * @param ipAddr  (IN) IP used for connection
 * @param curUser (IN) current user of application
 * @param appPort (IN) port of application
 *
 */
void cli_setAppData(char *appName, char *ipAddr, char *curUser, UINT16 appPort);


void cmd_install_cmddesc(const char *cmd, const char *desc, UTIL_VECTOR cmd_desc_vec);


VOS_RET_E CLI_addCommand(CLI_NODE_ID nodeId,
                         const char *cmd,
                         const char *help,
                         CLI_POWER_FUNC powerFunc,
                         CLI_FUNC func,
                         CLI_RUNTIME_FUNC runtimeFunc,
                         CLI_LEVEL level,
                         UBOOL8 hide);

void cmd_install_node(CMD_NODE_T *node, CLI_FUNC func);


void cmd_install_element(CLI_NODE_ID nodeid, CMD_ELEMENT_T *element);


void cmd_install_cmddesc(const char *cmd, const char *desc, UTIL_VECTOR cmd_desc_vec);


#endif /* __CLI_API_H__ */
