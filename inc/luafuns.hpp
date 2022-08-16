#ifndef _LUA_FUNS_H_
#define _LUA_FUNS_H_

#include "luaclass.hpp"
#include "tcp_server.h"

#define LUA_FILE_NAME "lua/lua_entry.lua"
#define LUA_RECEIVED_FROM_NETWORK "ReceivedFromNetwork"
#define LUA_SEND_TO_NETWORK "SendToNetwork"

int init_lua_code();
int received_from_network(char *r_buf, int sockfd, _logs log = LOG_OFF);
int send_to_network(lua_State *L);

#endif //_LUA_FUNS_H_
