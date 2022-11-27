#ifndef ELF_H
#define ELF_H

#include <apps.h>

/* ��� ��������� �������������� ����������. ��������� � ����� ���������� ������� */
typedef struct
{ 
    APPLICATION_T           apt; /* ������ ����������� ��������������, ������ ������ */
	
    /* ����� �������� ������ ������ ���������� */
    /* ... */

} APP_HELLOWORLD_T;

/* ��������� ���������� */
typedef enum
{
    HW_STATE_ANY, /* ANY-state ������ ������ */

    HW_STATE_MAIN,

    HW_STATE_MAX /* ��� �������� */
} HW_STATES_T;



UINT32 Register( char* file_uri,  char* param,  UINT32 reserve );

void My_APP_HandleEvent( EVENT_STACK_T *ev_st,
                         void *app,
                         APP_ID_T appid,
                         REG_ID_T regid );

UINT32 startApp( EVENT_STACK_T *ev_st,  REG_ID_T reg_id,  UINT32 param2 );
UINT32 destroyApp( EVENT_STACK_T *ev_st,  void *app );

UINT32 MainStateEnter( EVENT_STACK_T *ev_st,  void *app,  ENTER_STATE_TYPE_T type );
UINT32 HandleKeypress( EVENT_STACK_T *ev_st,  void *app );

/*
UINT32 OpenLog(WCHAR *uri);
UINT32 CloseLog(void);
UINT32 SendLog(char *str);
*/

#endif