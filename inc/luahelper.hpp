#ifndef _LUAHELPER_H_
#define _LUAHELPER_H_

#include<iostream>

#define DEBUG_MODE_ON 1
#define DEBUG_MODE_OFF 0

class Helper
{
	private:
	bool luaDebug = DEBUG_MODE_OFF;

	protected:
	void setDbgMode(bool onOff);
	void dbg(const char* str);
};

#endif //_LUAHELPER_H_

