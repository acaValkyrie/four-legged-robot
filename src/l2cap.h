#define HID_L2CAP_MESSAGE_SIZE  8
typedef void (*HID_L2CAP_CALLBACK)(uint8_t *p_msg); 

long hid_l2cap_initialize(HID_L2CAP_CALLBACK callback);
long hid_l2cap_connect(BD_ADDR addr);
long hid_l2cap_reconnect(void);
BT_STATUS hid_l2cap_is_connected(void);
