#pragma once

#define DECLARE_SINGLETON(type)public:          \
		static type** GetInstance()				\
		{										\
			static type* pInstance = new type;	\
			if(pInstance == NULL)				\
			  pInstance = new type;				\
			return &pInstance;					\
		}										\
		static void DestroyInstance()			\
		{										\
			type**	ppInstance = GetInstance();	\
			if(*ppInstance != NULL)				\
			{									\
				delete *ppInstance;				\
				*ppInstance = NULL;				\
			}									\
		}

#define GET_SINGLE(type) (*type::GetInstance())
#define DESTROY_SINGLE(type) (*type::GetInstance())->DestroyInstance()
#define  ERR_MSG(hWnd,sz) MessageBox(hWnd,sz,L"System Err",MB_OK)