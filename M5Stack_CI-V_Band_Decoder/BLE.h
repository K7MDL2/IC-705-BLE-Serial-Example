#ifndef BLE_H_
#define BLE_H_
#include "M5Stack_CI-V_Band_Decoder.h"
#ifndef M5STAMPC3U
void Scan_BLE_Servers(void);
static bool doConnect = false;
void BLE_loop(void);
void BLE_Setup(void);
#endif
#endif