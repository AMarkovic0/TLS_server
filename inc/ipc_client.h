#ifndef _IPC_CLIENT_H_
#define _IPC_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include<sys/types.h>
#include<sys/socket.h>
#include<sys/un.h>

#include<unistd.h>
#include<stdio.h>
#include<stdint.h>

#include"log.h"

#define ADDRESS	"tmp/IPC_SOCK.socket"

#define IPC_REPORT_ESTABLISHED (char*)"ConnectionEstablished"
#define IPC_REPORT_CLOSED (char*)"ConnectionClosed"

uint8_t ipc_client_init();
uint8_t ipc_client_connect();
ssize_t ipc_client_send(char *w_buf);
void ipc_client_close();

#ifdef __cplusplus
}
#endif

#endif //_IPC_CLIENT_H_
