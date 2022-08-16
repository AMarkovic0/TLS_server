#include "luahelper.hpp"

void Helper::setDbgMode(bool onOff)
{
	if(onOff)
		this->luaDebug = DEBUG_MODE_ON;
	else
		this->luaDebug = DEBUG_MODE_OFF;
}

void Helper::dbg(const char* str)
{
	if(this->luaDebug)
		std::cout << "DEBUG: " << str << std::endl;
}
