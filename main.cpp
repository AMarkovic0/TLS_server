#include "tcp_server.h"
#include "luafuns.hpp"

#define PORT 54321u

void* connection_handler(void *vargp);

//TODO: Implement closing signal handler

int main(int argc, char *argv[])
{
	pthread_t server_thread;

        init_lua_code();

	if(tcp_server_init(PORT, LOG_ON) == 0)
                tcp_server_listen(LOG_ON);
        else
                return -1;

	pthread_create(&server_thread, NULL, &connection_handler, NULL);
	pthread_join(server_thread, NULL);

	tcp_server_close(LOG_ON);

	return 0;
}

void read_callback(char *r_buf, int fd, _logs log)
{
	if(log == LOG_ON)
		printf("Received on file dec: %d content: %s\n", fd, r_buf);

        received_from_network(r_buf, fd);
	memset(r_buf, 0, sizeof(char)*BUF_SIZE);
}

void* connection_handler(void *vargp)
{
	char r_buf[BUF_SIZE];
	tcp_server_poll(r_buf, LOG_ON);
}
