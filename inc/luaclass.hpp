#ifndef _LUACLASS_HPP_G
#define _LUACLASS_HPP_

#include<lua.hpp>
#include<iostream>
#include<cstdarg>
#include<map>
#include<list>
#include<type_traits>

#include "luahelper.hpp"

#define LUA_OK 0
#define LUA_FAILED 1

#define TOP_OF_THE_STACK -1
#define CLEAN_THE_STACK 0
#define EMPTY_STRING std::string("")

typedef enum luaTypes
{
	INT = 1,
	FLOAT = 2,
	DOUBLE = 3,
	STRING = 4,
	BOOLEAN = 5
}luatype_t;

typedef std::list<luatype_t> luaTypeList_t;

class LuaFile
{
	public:
	std::string fileName;
	size_t outArgNum;
	luaTypeList_t outTypes;
	luaTypeList_t::iterator outTypesIt;

	LuaFile(std::string fileName, luaTypeList_t outTypes);
};

class LuaFun
{
	public:
	std::string funName;
	size_t inArgNum;
	size_t outArgNum;
	luaTypeList_t inTypes;
	luaTypeList_t::iterator inTypesIt;
	luaTypeList_t outTypes;
	luaTypeList_t::iterator outTypesIt;

	LuaFun(std::string funName, luaTypeList_t inTypes, luaTypeList_t outTypes);
};

typedef std::pair<std::string, LuaFile*> luaFileNamePair_t; // File Name vs File Instance
typedef std::map<std::string, LuaFile*> luaFileNameMap_t;
typedef std::pair<std::string, LuaFun*> luaFunctionNamePair_t; // Function Name vs Function Instance
typedef std::map<std::string, LuaFun*> luaFunctionNameMap_t;

/*TODO:
 * This class can not get a table as function or file return argument, that should be
 * implemented in the near future. I hope so...
 */
class Lua: protected Helper
{
	private:
	luaFileNameMap_t files;
	luaFileNameMap_t::iterator filesIt;
	luaFunctionNameMap_t functions;
	luaFunctionNameMap_t::iterator functionsIt;

	// This function is used just to get variable in the case of multiple return arguments
	void _getVar(int position, std::va_list& valist, luatype_t type);
	// This function is used just to set variable in the case of multiple input arguments
	void _setVar(std::va_list& valist, luatype_t type);

	public:
	lua_State *L;
	Lua(bool debug);
	virtual ~Lua();

	bool registerFile(std::string fileName, luaTypeList_t outTypes);
	bool registerFunction(std::string funName, luaTypeList_t inTypes, luaTypeList_t outTypes);

	int exeFile(std::string fileName, ...);
	int exeFun(std::string funName, ...);
	bool getVar(std::string varName, std::string& ret, int position = TOP_OF_THE_STACK);
	bool getVar(std::string varName, lua_CFunction& ret, int position = TOP_OF_THE_STACK);
	bool getVar(std::string varName, lua_State* ret, int position = TOP_OF_THE_STACK);
	bool getVar(std::string varName, bool& ret, int position = TOP_OF_THE_STACK);
	template<class T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
	bool getVar(std::string varName, T& ret, int position = TOP_OF_THE_STACK)
	{
		if(0 != varName.size())
			lua_getglobal(this->L, varName.c_str());
		if(lua_isnumber(this->L, position) || lua_isboolean (this->L, position))
		{
			this->dbg("Getting lua number from the stack");
			ret = lua_tonumber(this->L, position);
			return LUA_OK;
		}
		if(!lua_isuserdata(this->L, position))
		{
			// Not implemented
		}
		this->dbg("Lua stack does not contain number on the top. Can not get variable.");
		return LUA_FAILED;
	}
	void getTable(std::string tableName);
	template<class T, class U> void getTableValue(T key, U& val)
	{
		this->setVar(EMPTY_STRING, key);
		lua_gettable(this->L, -2);
		this->getVar(EMPTY_STRING, val);
		lua_settop(this->L, -2);
	}
	template<class T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
	void setVar(std::string varName, T& set)
	{
		this->dbg("Pushing number on the stack.");
		lua_pushnumber(this->L, set);
		if(0 != varName.size())
			lua_setglobal(this->L, varName.c_str());
		return;
	}
	void setVar(std::string varName, std::string& set);
	void setVar(std::string varName, lua_CFunction set);
	void setVar(std::string varName, bool& set);
	void makeTable(std::string tableName = EMPTY_STRING);
	template<class T, class U> void insertTable(T key, U val)
	{
		this->setVar(EMPTY_STRING, key);
		this->setVar(EMPTY_STRING, val);
		lua_settable(this->L, -3);
	}
	void setGlobalTable(std::string tableName);

	protected:
};

#endif // _LUACLASS_HPP_
