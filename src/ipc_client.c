#include"ipc_client.h"

static int ipc_sock;
static struct sockaddr_un *saun;

static int _ipc_client_init_socket()
{
	uint8_t ret;
	ipc_sock = socket(AF_UNIX, SOCK_SEQPACKET, 0);

	if (-1 == ipc_sock) {
		dbg("IPC client socket creation failed. \n");
		ret = -1;
	} else {
		dbg("IPC client socket creation successful. \n");
		ret = 0;
	}

	return ret;
}

uint8_t ipc_client_init()
{
	uint8_t ret = _ipc_client_init_socket();

	if(0 < ret) {
		saun->sun_family = AF_UNIX;
		strcpy(saun->sun_path, ADDRESS);
	}

	return ret;
}

uint8_t ipc_client_connect()
{
	uint8_t ret = -1;

	if(connect(ipc_sock, (const struct sockaddr*)saun, sizeof(saun)) < 0) {
		dbg("IPC client connection failed. \n");
	} else {
		dbg("IPC client connect successful. \n");
		ret = 0;
	}

	return ret;
}

ssize_t ipc_client_send(char *w_buf)
{
        return send(ipc_sock, w_buf, strlen(w_buf), 0);
}

void ipc_client_close()
{
	dbg("Closing IPC connection. \n");
	close(ipc_sock);
}
