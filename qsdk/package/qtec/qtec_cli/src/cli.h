#ifndef __CLI_H__
#define __CLI_H__


#include "fwk.h"

#define SUPPORT_CLI_CMD  1

/** Max length of a single line of input */
#define CLI_MAX_BUF_SZ   256

/** Number of seconds to sleep when max auth failures is reached. */
#define AUTH_FAIL_SLEEP  3

/** Permission bit for admin user */
#define PERM_ADMIN     0x80

/** Permission bit for support user */
#define PERM_SUPPORT   0x40

/** Permission bit for normal user (user) */
#define PERM_USER      0x01

/** Permission mask to indicate something that can be done by Admin only */
#define PERM1 (PERM_ADMIN)

/** Permission mask to indicate something that can be done by Admin or Support */
#define PERM2 (PERM_ADMIN|PERM_SUPPORT)

/** Permission mask to indicate something that can be done by Admin or Support or User */
#define PERM3 (PERM_ADMIN|PERM_SUPPORT|PERM_USER)

/** Max length of menuPathBuf */
#define MAX_MENU_PATH_LENGTH   1024

#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN             (6)
#endif

#define SERIAL_NUMBER_LEN         (8)

#define PASSWORD_LEN         (10)

#define MAX_ONT_IOMAP_NUM             (16)

#define EQUIPMENTID_LEN               (20)

#define ONTGVERSION_LEN               (16)

#define ONT_PRODUCT_CODE_UNKNOWN 0xFFFFFFFF;


/** The amount of time, in seconds, of idle-ness before timing out the CLI session. */
extern UINT32 exitOnIdleTimeout;

/** Global string buffer that displays where we are in the menu hierarchy. */
extern char menuPathBuf[MAX_MENU_PATH_LENGTH];

/** Global boolean to decide whether CLI should keep running. */
extern UBOOL8 cli_keepLooping;

/** Global var indicating who is logged in right now. Max length 63. */
extern char currUser[];

/** Global var indicating current permission level associated with current user. */
extern UINT8 currPerm;

/** Global pointer to message handle passed in by caller */
extern void *msgHandle;

/** Global var indicating current port */
extern UINT16 currAppPort;

/** Global var indicating current app */
extern char currAppName[];

/** Global var indicating current IP connection */
extern char currIpAddr[];

/** Global var used in cli_menu to tell post-func to save config */
extern UBOOL8 cliMenuSaveNeeded;

/** Global var used in cli_cmd to save config */
extern UBOOL8 cliCmdSaveNeeded;


/** used in PARAMS_SETTING in cli_menu */
typedef UBOOL8(*CLI_VALIDATE_FUNC) (const char *inputParam);

/** this is only used in cli_menu */
typedef struct
{
    char *prompt;
    char param[CLI_MAX_BUF_SZ];
    CLI_VALIDATE_FUNC validateFnc;
} PARAMS_SETTING;


typedef enum
{
    BOOTTYPE_UNKOWN                     = 0xFFFFFFFF, 
 
}BOOT_TYPE_E;

typedef enum
{
    HW_GPIO_01    = 1,
    HW_GPIO_02    = 2,
    HW_GPIO_03    = 3,
    HW_GPIO_04    = 4,
    HW_GPIO_05    = 5,
    HW_GPIO_06    = 6,
    HW_GPIO_07    = 7,
    HW_GPIO_08    = 8,
 
}GPIO_VERSION_E;


typedef struct
{
  UINT32 codeClass;
  UINT32 codeHwInfo;
  UINT32 codePortMap;
  UINT32 codeExtend;
}ONT_PRODUCT_CODE_T;

typedef struct
{
    char loid[25];
    char password[13];
    char sn[130];
}ONT_CTC_T;

typedef struct
{
    unsigned char addr[MAC_ADDR_LEN];
}MAC_ADDR;

typedef struct
{
    unsigned char sn[SERIAL_NUMBER_LEN];
}SERIAL_NUMBER;

typedef struct
{
    unsigned char pwd[PASSWORD_LEN];
}PON_PASSWORD;

typedef struct
{
    UINT8    isValid:1;
    UINT8    type:3; // ONT_IOTYPE_GPIO ...
    UINT8    reservedBit:4;
    UINT8    reserved[3];
    UINT32   addr;
    UINT32   data;
} ONT_IOMAP_INFO_T;

typedef struct
{
    UINT32                   crc;
    BOOT_TYPE_E              bootType;
    GPIO_VERSION_E           gpioVersion;
    MAC_ADDR                 mac1;   //voipMac
    MAC_ADDR                 mac2;   //uniMac
    UINT32                   ipaddr;
    UINT32                   gateway;
    UINT32                   netMask; 
    SERIAL_NUMBER            sn;
    ONT_PRODUCT_CODE_T         productCode;    
    PON_PASSWORD             password;
    UINT8                    reserved[2];
    MAC_ADDR                 mac3;   //ponMac
    ONT_CTC_T                ctcLoid;
    
    ONT_IOMAP_INFO_T         ioMap[MAX_ONT_IOMAP_NUM];
    char                     eqptId[EQUIPMENTID_LEN];
    char                     ontgVer[ONTGVERSION_LEN];
}ONT_INFO_T;


/** Display the current menu.
 *
 */
void cli_displayMenu(void);


/** Check cmdLine against the menu driven table.
 *
 * @param cmdLine (IN) command line from user.
 *
 * @return TRUE if cmdLine was a menu driven item that was processed by
 * this function.
 */
UBOOL8 cli_processMenuItem(const char *cmdLine);


/** Check cmdLine against the CLI cmd table.
 *
 * @param cmdLine (IN) command line from user.
 *
 * @return TRUE if cmdLine was a CLI command that was processed by
 * this function.
 */
UBOOL8 cli_processCliCmd(const char *cmdLine);


/** Check cmdLine against the hidden cmd table.
 *
 * @param cmdLine (IN) command line from user.
 *
 * @return TRUE if cmdLine was a hidden command that was processed by
 * this function.
 */
UBOOL8 cli_processHiddenCmd(const char *cmdLine);


/** Wait for user input before continuing.
 *  This is used when menu driven CLI mode is enabled.
 */
void cli_waitForContinue(void);


/** Wait for the specified amount of time for input to be available on stdin.
 *  This function will also return if a signal was received.
 *
 *
 * @return VOS_RET_SUCCESS if input becomes available.
 *         VOS_RET_TIMED_OUT if user stops typing for the exit-on-idle number of seconds.
 *         VOS_RET_OP_INTR if input was interrupted by a signal.
 */
VOS_RET_E cli_waitForInputAvailable(void);

/** Read a line from standard input.
 *
 * Note this function calls cli_waitForInputAvailable() prior to calling the
 * read, which blocks.
 * 
 * @param buf (OUT) buffer to hold the text read.
 * @param size (IN) Size of the buffer.
 *
 * @return VOS_RET_SUCCESS if input was read.
 *         VOS_RET_TIMED_OUT if user stops typing for the exit-on-idle number of seconds.
 *         VOS_RET_OP_INTR if input was interrupted by a signal.
 */
VOS_RET_E cli_readString(char *buf, int size);


/** Return true if the specified state is valid.
 *  For menu cli, valid state is 1 for on/enabled, and 2 for off/disabled.
 *
 * @param state (IN) state string.
 *
 * @return TRUE if the state string is valid.
 */
UBOOL8 cli_isValidState(const char *state);


#endif /* __CLI_H__ */
