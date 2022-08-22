#include "tcp_server.h"
#include "luafuns.hpp"

#define PORT 54321u

_logs log = LOG_ON;
void* connection_handler(void *vargp);

//TODO: Implement closing signal handler

int main(int argc, char *argv[])
{
	pthread_t server_thread;

        init_lua_code();

	if(tcp_server_init(PORT) == 0)
                tcp_server_listen();
        else
                return -1;

	pthread_create(&server_thread, NULL, &connection_handler, NULL);
	pthread_join(server_thread, NULL);

	tcp_server_close();

	return 0;
}

void read_callback(char *r_buf, int fd)
{
        dbg("Received on file dec: %d content: %s\n", fd, r_buf);

        received_from_network(r_buf, fd);
	memset(r_buf, 0, sizeof(char)*BUF_SIZE);
}

void* connection_handler(void *vargp)
{
	char r_buf[BUF_SIZE];
	tcp_server_poll(r_buf);
}
