#ifndef BLE_H_
#define BLE_H_

void Scan_BLE_Servers(void);
static bool doConnect = false;
void BLE_loop(void);
void BLE_Setup(void);

#endif