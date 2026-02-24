#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

bool is_in_whitelist(uint8_t *bssid);
void start_web_interface();
void web_interface_handle_client();

#endif