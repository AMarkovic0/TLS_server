#include "luafuns.hpp"

static Lua lua_stack = Lua(DEBUG_MODE_OFF);

int init_lua_code()
{
	int luafile_check = 0;

        lua_stack.registerFile(LUA_FILE_NAME, {INT});
	lua_stack.registerFunction(LUA_RECEIVED_FROM_NETWORK, {STRING, INT}, {INT});

        lua_stack.setVar(std::string(LUA_SEND_TO_NETWORK), send_to_network);
        lua_stack.setVar(std::string(LUA_SEND_TO_NETWORK_SSL), send_to_network_ssl);
        lua_stack.setVar(std::string(LUA_CLOSE_CONNECTION), close_connection);

	if(EXIT_SUCCESS != lua_stack.exeFile(LUA_FILE_NAME, &luafile_check)) {
		printf("LUA ERROR: Lua file %s failed to execute. \n", LUA_FILE_NAME);
	}

        return luafile_check;
}

int received_from_network(char *r_buf, int sockfd, _logs log)
{
        int ret;

        if(EXIT_SUCCESS != lua_stack.exeFun(LUA_RECEIVED_FROM_NETWORK, std::string(r_buf), &sockfd, &ret)) {
		if(log == LOG_ON)
			printf("LUA ERROR: %s failed to execute. \n", LUA_RECEIVED_FROM_NETWORK);
	}
        return ret;
}

int send_to_network(lua_State *L)
{
        int sockfd;
        std::string w_buf;

        lua_stack.getVar(EMPTY_STRING, w_buf, 1);
        lua_stack.getVar(EMPTY_STRING, sockfd, 2);

        return tcp_server_send(sockfd, &w_buf[0]);
}

int send_to_network_ssl(lua_State *L)
{
        SSL *ssl;
        int sockfd;
        std::string w_buf;

        lua_stack.getVar(EMPTY_STRING, w_buf, 1);
        lua_stack.getVar(EMPTY_STRING, sockfd, 2);

        ssl = tcp_server_get_ssl();

        return tcp_server_ssl_send(ssl, &w_buf[0]);
}

int close_connection(lua_State *L)
{
	int sockfd;

        lua_stack.getVar(EMPTY_STRING, sockfd, 1);
	tcp_server_close_connection(sockfd);
	return 0;
}
