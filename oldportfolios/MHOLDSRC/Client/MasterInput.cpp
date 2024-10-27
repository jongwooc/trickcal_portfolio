#include "StdAfx.h"
#include "MasterInput.h"



void CMasterInput::Process()
{
	InputAdapter = 0;

	if(GetAsyncKeyState(VK_LBUTTON))
	{	InputAdapter = KEY_LBUTTON;	}
	if(GetAsyncKeyState(VK_RBUTTON))
	{	InputAdapter = KEY_RBUTTON;	}
	if(GetAsyncKeyState(VK_LEFT))
	{	InputAdapter = KEY_LEFT;	}
	if(GetAsyncKeyState(VK_UP))
	{	InputAdapter = KEY_UP;		}
	if(GetAsyncKeyState(VK_RIGHT))
	{	InputAdapter = KEY_RIGHT;	}
	if(GetAsyncKeyState(VK_DOWN))
	{	InputAdapter = KEY_DOWN;	}
	if(GetAsyncKeyState(VK_LEFT) && GetAsyncKeyState(VK_UP))
	{	InputAdapter = KEY_LEFT + KEY_UP;	}
	if(GetAsyncKeyState(VK_UP) && GetAsyncKeyState(VK_RIGHT))
	{	InputAdapter = KEY_UP + KEY_RIGHT;	}
	if(GetAsyncKeyState(VK_RIGHT) && GetAsyncKeyState(VK_DOWN))
	{	InputAdapter = KEY_RIGHT + KEY_DOWN;	}
	if(GetAsyncKeyState(VK_DOWN) && GetAsyncKeyState(VK_LEFT))
	{	InputAdapter = KEY_DOWN + KEY_LEFT;	}





	GET_SINGLE(CSceneMgr)->Progress(InputAdapter);

}

CMasterInput::CMasterInput(void)
{
}


CMasterInput::~CMasterInput(void)
{
}
