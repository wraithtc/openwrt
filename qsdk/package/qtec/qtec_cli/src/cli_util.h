#ifndef __CLI_UTIL_H__
#define __CLI_UTIL_H__


#include "fwk.h"
#include "cli_api.h"
#include "cli_vty.h"
#include "cli_cmd.h"



VOS_RET_E cli_handleSelf(CMD_ELEMENT_T *element, VTY_T *vty, int argc, char **argv);


VOS_RET_E CLI_addNode(CLI_NODE_ID parentNodeId, CLI_NODE_ID nodeId, const char *prompt, CLI_FUNC func);

VOS_RET_E CLI_handleRemoteExecute(VosEntityId eid,
                                  VosMsgType msgType,
                                  int argc,
                                  char **argv);

VOS_RET_E CLI_handleRemoteRuntime(VosEntityId eid,
                                  VosMsgType msgType,
                                  int argc,
                                  char **argv,
                                  UTIL_VECTOR vecCmdDesc);

VOS_RET_E CLI_handleShowTr69Soap(VosEntityId eid,
                                 VosMsgType msgType,
                                 int argc,
                                 char **argv,
                                 UTIL_VECTOR vecCmdDesc);


#endif /* __CLI_UTIL_H__ */
