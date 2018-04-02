#ifndef LIBRTCFG_H
#define LIBRTCFG_H

#include <stdlib.h>
#include "wireless_set.h"
#include "lan_set.h"
#include "network_set.h"
#include "lan_set.h"
#include "ntp_set.h"
#include "auth_set.h"
#include "firewall_set.h"
#include "systeminfo_get.h"
#include "lanhost.h"
#include "devinfo.h"
#include "timerTask_set.h"
#include "system_set.h"
#include "offline_get.h"
#include "arpband_set.h"
#include "rtcfg_uci.h"
#include "security.h"
#include "vpn.h"
#include "wifi_txpower.h"
#include "manufactory.h"
#include "fwk.h"

typedef enum _WanAct
{
	WANNONE,
	WANDHCP,
	WANPPPOE,
	WANSTATIC
}WanAct;

typedef enum _LanAct
{
	LANNONE,
	LANSET,
	LANGET,
	LANADDSTATIC,
	LANDELSTATIC,
	LANGETSTATIC
}LanAct;

typedef enum _PortForwardAct
{
	PFNONE,
	PFADD,
	PFDEL,
	PFEDIT,
	PFGET
}PortForwardAct;

typedef enum _NtpAct
{
	NTPNONE,
	NTPGET,
	NTPSET
}NtpAct;

typedef enum _WifiAct
{
	WIFINONE,
	WIFIADD,
	WIFIDEL,
	WIFIEDIT,
	WIFIGET
}WifiAct;


#define DEBUG 1
#ifdef DEBUG
#define DEBUG_PRINTF(format,...) printf(format, ##__VA_ARGS__)
#else
#define DEBUG_PRINTF(format,...)
#endif 
#endif
