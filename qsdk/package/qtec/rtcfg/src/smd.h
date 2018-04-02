#ifndef __SMD_H__
#define __SMD_H__

/** Max number of milliseconds between the communications socket connect and
 *  getting the APP_LAUNCHED message.
 */
#define SMD_CONNECT_TO_LAUNCH_MSG_TIMEOUT    (200)
#define SMD_LCK_FUNC_NAME_LENGTH   BUFLEN_64
/** Maximum length of optional arguments to a dynamically launched app. */
#define SMD_DLS_OPT_ARGS_LENGTH    (256)

#define SMD_ZOMBIE_CHECK_INTERVAL    (30 * MSECS_IN_SEC)

/*!\enum SMD_DLS_STATE_E
 * \brief States of a dynamically launched process, used in SMD_DLS_INFO_T.state.
 */
typedef enum
{
    DLS_NOT_RUNNING = 0, /**< not running */
    DLS_LAUNCHED = 1,    /**< launched, but waiting for confirmation */
    DLS_RUNNING = 2,     /**< fully up and running. */
    DLS_TERMINATE_REQUESTED = 3 /**< Requested termination, but waiting for confirmation. */
} SMD_DLS_STATE_E;

typedef enum
{
    HEARTBEAT_INIT,
    HEARTBEAT_RESET,
    HEARTBEAT_STOP,
} SMD_HEARTBEAT_E;

typedef struct
{
    DlistNode dlist;        /** handle for doubly-linked list */
    const VosEntityInfo *eInfo;   /**< Pointer to entity info entry */
    SMD_DLS_STATE_E state;         /**< Current state of this dynamically launched process. */
    SINT32 serverFd;        /**< Server/listening socket fd */
    SINT32 commFd;          /**< Unix domain TCP connection socket file
                             *   descriptor on the smd process for
                             * inter-process communication with the
                             * application process.
                             */
    SINT32 pid;                  /**< pid of the app */
    UINT32 specificEid;          /**< if this app can have multiple instances, combined pid and eid */
    SINT32 numDelayedMsgRequested;         /**< number of delayed msgs requested by this app */
    SINT32 numEventInterestRequested;         /**< number of event interests requested by this app */
    VosMsgHeader *msgQueue;  /**< queue of messages waiting to be delivered to this app. */
    char optArgs[SMD_DLS_OPT_ARGS_LENGTH];   /**< additional dynamic args, usually sent in by restart msg */
    SMD_HEARTBEAT_E heartbeat;
} SMD_DLS_INFO_T;

/** Structure to track a single entity that is interested in a particular event.
 *
 */
typedef struct event_interest_info
{
    DlistNode dlist;             /**< generic doubly linked list handle */
    VosEntityId eid;             /**< eid of interested app. */
    char *matchData;             /**< extra matching data beyond just the msgType. */
} SMD_EVT_INTEREST_INFO_T;


/** Structure to keep track of all entities that are interested in a particular event.
 */
typedef struct event_interest
{
    DlistNode dlist;           /**< generic doubly linked list handle */
    VosMsgType type;           /**< Event of interest, which is just a notification msg type */
    DlistNode evtInfoHead;     /**< Doubly linked list of SMD_EVT_INTEREST_INFO_T's for this event. */
} SMD_EVT_INTEREST_T;

typedef struct
{
    char *stringsStart;         /**< Start of strings area. */
    void *mallocStart;          /**< Start of region used for shared memory alloc. */

    UBOOL8 locked;              /**< TRUE if a process has the lock. */
    SINT32 lockOwner;           /**< Pid of current lock owner. */
    UtilTimestamp timeAquired;    /**< Time stamp of when lock was aquired. */
    char callerFuncName[SMD_LCK_FUNC_NAME_LENGTH];          /**< Function name of caller who aquired the lock. */
} SMD_LOCK_CONTEXT;

void smd_sendMessageByState(SMD_DLS_INFO_T * dInfo, VosMsgHeader ** msg);
VOS_RET_E smd_lockInit(UBOOL8 attachExisting);
void smd_LockCleanup(void);
VOS_RET_E smd_Lock(const UINT32 * timeoutMs, SMD_DLS_INFO_T * dInfo);
VOS_RET_E smd_Unlock();
#endif