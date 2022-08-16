#include "tcp_server.h"
#include "luaclass.hpp"

#define PORT 54321u
#define LUA_FD 1
#define NUM_OF_LUA_STACKS 10
#define LUA_FILE_NAME "handler.lua"
#define LUA_RECEIVED_FROM_NETWORK "ReceivedFromNetwork"

void* connection_handler(void *vargp);

Lua lua_stack = Lua(DEBUG_MODE_OFF);

int main(int argc, char *argv[])
{
	int luafile_check = 0;
	pthread_t server_thread;

	lua_stack.registerFile(LUA_FILE_NAME, {INT});
	lua_stack.registerFunction(LUA_RECEIVED_FROM_NETWORK, {STRING}, {INT});
	if(EXIT_SUCCESS != lua_stack.exeFile(LUA_FILE_NAME, &luafile_check)) {
		printf("LUA ERROR: Lua file %s failed to execute. \n", LUA_FILE_NAME);
	}

	tcp_server_init(PORT, LOG_ON);
	tcp_server_listen(LOG_ON);

	pthread_create(&server_thread, NULL, &connection_handler, NULL);
	pthread_join(server_thread, NULL);

	tcp_server_close(LOG_ON);

	return 0;
}

void read_callback(char* r_buf, int fd, _logs log)
{
	int ret = 0;

	if(log == LOG_ON)
		printf("Received on file dec: %d content: %s\n", fd, r_buf);

	if(EXIT_SUCCESS != lua_stack.exeFun(LUA_RECEIVED_FROM_NETWORK, std::string(r_buf), &ret)) {
		if(log == LOG_ON)
			printf("LUA ERROR: %s failed to execute. \n", LUA_RECEIVED_FROM_NETWORK);
	}

	memset(r_buf, 0, sizeof(char)*BUF_SIZE);
	ret = 0;
}

void* connection_handler(void *vargp)
{
	char r_buf[BUF_SIZE];
	tcp_server_poll(r_buf, LOG_ON);
}
