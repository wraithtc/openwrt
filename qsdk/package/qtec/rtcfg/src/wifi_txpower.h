#ifndef __WIFI_TXPOWER_H__
#define __WIFI_TXPOWER_H__

#define TXPOWER_2_4_VALUE_FILE    "/tmp/txpower_2_4_value"
#define TXPOWER_5_VALUE_FILE    "/tmp/txpower_5_value"
#define TXPOWER_CONFIG_FILE    "/etc/config/wifiTxPower"
#define MAX(a,b) (a)>(b)?(a):(b)

typedef enum{
    TXPOWER_MODE_LOW = 0,
    TXPOWER_MODE_MEDIUM,
    TXPOWER_MODE_HIGH,
    TXPOWER_MODE_BUTTOM
}TXPOWER_MODE_E;

int QtSetWifTxpower(int mode);

int QtGetWifiMode(int* mode);

#endif /* __WIFI_TXPOWER_H__ */