#include "luaclass.hpp"

LuaFile::LuaFile(std::string fileName, luaTypeList_t outTypes)
{
	this->fileName = fileName;
	this->outArgNum = outTypes.size();
	this->outTypes = outTypes;
}

LuaFun::LuaFun(std::string funName, luaTypeList_t inTypes, luaTypeList_t outTypes)
{
	this->funName = funName;
	this->inArgNum = inTypes.size();
	this->outArgNum = outTypes.size();
	this->inTypes = inTypes;
	this->outTypes = outTypes;
}

Lua::Lua(bool debug = DEBUG_MODE_OFF)
{
	this->setDbgMode(debug);
	this->L = luaL_newstate();
	this->dbg("Lua VM has been created");
	luaL_openlibs(this->L);
	this->dbg("Lua standard library has been loaded");
}

Lua::~Lua()
{
	dbg("Destroying all object in the Lua state. Closing Lua state.");
	lua_close(this->L);
}

bool Lua::registerFile(std::string fileName, luaTypeList_t outTypes)
{
	this->files[fileName] = new LuaFile(fileName, outTypes);
}

bool Lua::registerFunction(std::string funName, luaTypeList_t inTypes, luaTypeList_t outTypes)
{
	this->functions[funName] = new LuaFun(funName, inTypes, outTypes);
}

int Lua::exeFile(std::string fileName, ...)
{
	std::va_list valist;
	int res;

	// Execute
	LuaFile* file = this->files.find(fileName)->second;
	if(EXIT_SUCCESS == luaL_dofile(this->L, fileName.c_str()))
	{
		this->dbg("File has been executed.");
	}
	else
	{
		this->dbg("File execution failed");
		res = lua_tonumber(this->L, TOP_OF_THE_STACK); // Getting error message
		lua_settop (this->L, CLEAN_THE_STACK);
		return res;
	}

	// Get output arguments
	va_start(valist, fileName);
	file->outTypesIt = file->outTypes.begin();
	for(int cnt = -(file->outArgNum); cnt < 0; cnt++)
	{
		std::advance(file->outTypesIt, cnt + file->outArgNum);
		this->_getVar(cnt, valist, *(file->outTypesIt));
		file->outTypesIt = file->outTypes.begin();
	}
	va_end(valist);
	return EXIT_SUCCESS;
}

int Lua::exeFun(std::string funName, ...)
{
	std::va_list valist;
	int res;

	LuaFun* function = this->functions.find(funName)->second;
	lua_getglobal(L, funName.c_str()); // Push the function to the Lua stack

	// Push input arguments
	va_start(valist, funName);
	function->inTypesIt = function->inTypes.begin();
	for(int cnt = 0; cnt < function->inArgNum; cnt++)
	{
		std::advance(function->inTypesIt, cnt);
		this->_setVar(valist, *(function->inTypesIt));
		function->inTypesIt = function->inTypes.begin();
	}

	// Execute
	if(EXIT_SUCCESS == lua_pcall(L, function->inArgNum, function->outArgNum, 0))
	{
		this->dbg("Function has been executed.");
	}
	else
	{
		this->dbg("Function execution failed.");
		res = lua_tonumber(this->L, TOP_OF_THE_STACK); // Getting error message
		lua_settop (this->L, CLEAN_THE_STACK);
		return res;
	}

	// Get output arguments
	function->outTypesIt = function->outTypes.begin();
	for(int cnt = -(function->outArgNum); cnt < 0; cnt++)
	{
		std::advance(function->outTypesIt, cnt + function->outArgNum);
		this->_getVar(cnt, valist, *(function->outTypesIt));
		function->outTypesIt = function->outTypes.begin();
	}
	va_end(valist);
	return EXIT_SUCCESS;
}

bool Lua::getVar(std::string varName, std::string& ret, int position)
{
	if (0 != varName.size())
		lua_getglobal(this->L, varName.c_str());
	if(lua_isstring(this->L, position))
	{
		this->dbg("Getting lua string from the stack");
		ret = std::string(lua_tolstring(this->L, position, NULL));
		return LUA_OK;
	}
	this->dbg("Lua stack does not contain string on the top. Can not get variable.");
	return LUA_FAILED;
}

bool Lua::getVar(std::string varName, lua_CFunction& ret, int position)
{
	if(0 != varName.size())
		lua_getglobal(this->L, varName.c_str());
	if(lua_iscfunction(this->L, position))
	{
		this->dbg("Getting lua C function from the stack");
		ret = lua_tocfunction(this->L, position);
		return LUA_OK;
	}
	this->dbg("Lua stack does not contain C function on the top. Can not get variable.");
	return LUA_FAILED;
}

bool Lua::getVar(std::string varName, lua_State* ret, int position)
{
	if(0 != varName.size())
		lua_getglobal(this->L, varName.c_str());
	if(lua_isthread(this->L, position))
	{
		this->dbg("Getting lua thread from the stack");
		ret = lua_tothread(this->L, position);
		return LUA_OK;
	}
	this->dbg("Lua stack does not contain thread on the top. Can not get variable.");
	return LUA_FAILED;
}

bool Lua::getVar(std::string varName, bool& ret, int position)
{
	if(0 != varName.size())
		lua_getglobal(this->L, varName.c_str());
	if(lua_isboolean(this->L, position) || lua_isnoneornil(this->L, position))
	{
		this->dbg("Getting lua boolean from the stack");
		ret = lua_toboolean(this->L, position);
		return LUA_OK;
	}
	this->dbg("Lua stack does not contain boolean on the top. Can not get variable.");
	return LUA_FAILED;
}

void Lua::getTable(std::string tableName)
{
	lua_getglobal(this->L, tableName.c_str());
}

void Lua::setVar(std::string varName, std::string& set)
{
	this->dbg("Pushing string on the stack.");
	lua_pushstring(this->L, set.c_str());
	if(0 != varName.size())
		lua_setglobal(this->L, varName.c_str());
	return;
}

void Lua::setVar(std::string varName, lua_CFunction set)
{
	this->dbg("Pushing C function on the stack.");
	lua_pushcfunction(this->L, set); // Check how it is done
	if(0 != varName.size())
		lua_setglobal(this->L, varName.c_str());
	return;
}

void Lua::setVar(std::string varName, bool& set)
{
	this->dbg("Pushing boolean on the stack.");
	lua_pushboolean(this->L, set);
	if(0 != varName.size())
		lua_setglobal(this->L, varName.c_str());
	return;
}

void Lua::makeTable(std::string tableName)
{
	this->dbg("Table has been made.");
	lua_newtable(this->L);
	if(0 != tableName.size())
		lua_setglobal(this->L, tableName.c_str());
	return;
}

void Lua::setGlobalTable(std::string tableName)
{
	lua_setglobal(this->L, tableName.c_str());
}

void Lua::_getVar(int position, std::va_list& valist, luatype_t type)
{
	if(lua_isnumber(this->L, position) || lua_isboolean (this->L, position))
	{
		if(type == INT)
		{
			*(va_arg(valist, int*)) = lua_tonumber(this->L, position);
			return;
		}
		if(type == FLOAT)
		{
			*(va_arg(valist, float*)) = lua_tonumber(this->L, position);
			return;
		}
		if(type == DOUBLE)
		{
			*(va_arg(valist, double*)) = lua_tonumber(this->L, position);
			return;
		}
	}
	if(lua_isfunction(this->L, position))
	{
		// Not implemented
	}
	if(lua_isnoneornil(this->L, position))
	{
		dbg("None or Nil value read from the Lua stack.");
		return;
	}
	if(lua_isstring(this->L, position))
	{
		return;
	}
	if(lua_istable(this->L, position))
	{
		return;
	}
	if(lua_isthread(this->L, position))
	{
		// Not implemented
	}
	if(!lua_isuserdata(this->L, position))
	{
		// Not implemented
	}
}

void Lua::_setVar(std::va_list& valist, luatype_t type)
{
	if(type == INT)
	{
		lua_pushnumber(L, *(va_arg(valist, int*)));
		return;
	}
	if(type == FLOAT)
	{
		lua_pushnumber(L, *(va_arg(valist, float*)));
		return;
	}
	if(type == DOUBLE)
	{
		lua_pushnumber(L, *(va_arg(valist, double*)));
		return;
	}
	if(type == STRING)
	{
		lua_pushstring(L, va_arg(valist, std::string).c_str());
		return;
	}
	if(type == BOOLEAN)
	{
		return;
	}
}
