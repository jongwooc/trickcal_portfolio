#pragma once

template<typename T>
void SAFE_DELETE(T& Pointer)
{
	if(Pointer)
	{
		delete Pointer;
		Pointer = NULL;
	}
}
template<typename T>
void SAFE_DELETE_ARRAY(T& Pointer)
{
	if(Pointer)
	{
		delete[] Pointer;
		Pointer = NULL;
	}
}