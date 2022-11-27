#ifndef ELF_H
#define ELF_H

#include <apps.h>

/* Эта структура идентифицирует приложение. Передаётся в любой обработчик событий */
typedef struct
{ 
    APPLICATION_T           apt; /* Должно обязательно присутствовать, причём первым */
	
    /* Здесь кладутся прочие данные приложения */
    /* ... */

} APP_HELLOWORLD_T;

/* Состояния приложения */
typedef enum
{
    HW_STATE_ANY, /* ANY-state всегда первым */

    HW_STATE_MAIN,

    HW_STATE_MAX /* Для удобства */
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