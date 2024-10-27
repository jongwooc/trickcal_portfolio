#pragma once
#include "Include.h"

class CMasterInput
{
private :
	DWORD InputAdapter;
public:
	DECLARE_SINGLETON(CMasterInput);
	void Initialize(){InputAdapter = 0x00000000;}
	void Process();

public:
	CMasterInput(void);
	~CMasterInput(void);
};

