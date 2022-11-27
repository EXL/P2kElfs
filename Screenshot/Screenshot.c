#include "Screenshot.h"
#include <dbg.h>

#include <ati.h>

#define HAPI_BATTERY_ROM_BYTE_SIZE 128
#define HAPI_BATTERY_ROM_UNIQUE_ID_SIZE 6

extern UINT16 display_source_buffer[];

#define BMP_HEADER_SIZE 70
static UINT8 BMP_HEADER[BMP_HEADER_SIZE] = {	
	0x42, 0x4D, 0x46, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00,
	0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xA0, 0x00,
	0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0xA0,
	0x00, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07,
	0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

enum
{
    HAPI_BATTERY_ROM_NONE = 0,
    HAPI_BATTERY_ROM_W_DATA,
    HAPI_BATTERY_ROM_WO_DATA,
    HAPI_BATTERY_ROM_INVALID
};

typedef UINT8 HAPI_BATTERY_ROM_T;

HAPI_BATTERY_ROM_T HAPI_BATTERY_ROM_read( UINT8 *dest_address);

void HAPI_BATTERY_ROM_get_unique_id(UINT8 *unique_id);

/* Название приложения. Длина строки именно такая и никакая иначе */
const char app_name[APP_NAME_LEN] = "EventsLog"; 

WCHAR *uri = L"/a/batt_rom.dump";

UINT32 	F1=0x00000000; // фильтры 
UINT32 	F2=0xFFFFFFFF;

FILE LogFile=NULL;

BOOL	destroy=false;

static WCHAR path[FILEURI_MAX_LEN];

// Обработчики событий для каждого state-а

/* Обработчики событий для HW_STATE_ANY (используется в любом state) */
const EVENT_HANDLER_ENTRY_T any_state_handlers[] =
{  
    { EV_KEY_PRESS,             HandleKeypress },


    // Список всегда должен заканчиватся такой записью
    { STATE_HANDLERS_END,           NULL           },
};

const EVENT_HANDLER_ENTRY_T main_state_handlers[] =
{
    { STATE_HANDLERS_END,           NULL           },
};

/* Таблица соответствий обработчиков, состояний и функций входа-выхода из состояния.
   Порядок состояний такой же, как в enum-e */
static const STATE_HANDLERS_ENTRY_T state_handling_table[] =
{
    { HW_STATE_ANY,               // State
      NULL,                       // Обработчик входа в state
      NULL,                       // Обработчик выхода из state
      any_state_handlers          // Список обработчиков событий
    },

    { HW_STATE_MAIN,
      MainStateEnter,
      NULL,
      main_state_handlers
    }

};


/* Это entry для всех эльфов, должна быть объявлена именно так */
/* file_uri - путь к эльфу (аналогично argv[0]) */
/* param - параметры эльфа (аналогично остальным argv) */
/* reserve - выделенная база eventcode-ов (зарезервировано 64 значения)*/
UINT32 Register( char* file_uri,  char* param,  UINT32 reserve )
{
    UINT32 status = RESULT_OK;
	UINT32 evcode_base= reserve;    // Сохраняем reserve
	
	WCHAR nameCfgFile[64];
	
	UINT32 bufR[2];
	UINT32 readen;
	FILE_HANDLE_T f;
    
	

/* Регистрируем приложение */
    status = APP_Register( &evcode_base,            // На какое событие должно запускаться приложение
                           1,                       // Кол-во событий, на которое должно запускаться приложение
                           state_handling_table,    // Таблица состояний
                           HW_STATE_MAX,            // Количество состояний
                           (void*)startApp );   // Функция, вызываемая при запуске
                   
	// определяем имя конфига
	/*
		u_atou(file_uri, nameCfgFile);
		nameCfgFile[u_strlen(nameCfgFile)-3] = 0;
		u_strcat(nameCfgFile, L"cfg");			

		udbg("nameCfgFile = %s\n", nameCfgFile);		
		
		// читаем данные из конфига		
		if (DL_FsFFileExist( nameCfgFile)) // проверяем есть ли файл
		{
			f = DL_FsOpenFile(nameCfgFile, FILE_READ_MODE, 0);
			DL_FsReadFile( bufR, sizeof(UINT32)*2, 1, f, &readen );
			DL_FsCloseFile(f);
			
				F1 = bufR[0];
				F2 = bufR[1];
				dbg("Filtr1 = 0x%x\n", F1);
				dbg("Filtr2 = 0x%x\n", F2);
		}	
		
		// создаёт лог файл
		nameCfgFile[u_strlen(nameCfgFile)-3] = 0;
		u_strcat(nameCfgFile, L"log");
		OpenLog(nameCfgFile);
	*/	
    LdrStartApp(reserve);   // Запустить немедленно
    
    return 1;   // Пока не имеет значения, что возвращать
}

void My_APP_HandleEvent( EVENT_STACK_T *ev_st, void *app, APP_ID_T appid, REG_ID_T regid )
{
	char event_str[32];
	EVENT_T  *event = AFW_GetEv(ev_st);
	
#if 0
	if (event->code >= F1 && event->code <= F2) {
		if(event->att_size > 0) 
			PFprintf("EVENT = 0x%08X %d 0x%08X att_size=%d\n", event->code, event->code, event->data, event->att_size);
		else 
			PFprintf("EVENT = 0x%08X %d 0x%08X\n", event->code, event->code, event->data);
		/*strcpy(event_str, "0x"); itoa(event->code, event_str+strlen(event_str), 16); 
		if (event->att_size > 0) { // есть ли аттач?
			strcat(event_str, "   att_size="); itoa(event->att_size, event_str+strlen(event_str), 10); strcat(event_str, " b");
		}
		strcat(event_str, "\r\n");
		SendLog(event_str);
		*/
	}
#endif
    APP_HandleEventPrepost( ev_st,  app,  appid,  regid );
}


/* Функция вызываемая при старте приложения */
UINT32 startApp( EVENT_STACK_T *ev_st,  REG_ID_T reg_id,  UINT32 param2 )//void *reg_hdl )
{
    APP_HELLOWORLD_T     *app = NULL;
    UINT32 status = RESULT_OK;
    


    /* Проверяем, а вдруг уже запущено? */
    if( AFW_InquireRoutingStackByRegId( reg_id ) == RESULT_OK )
    {
        return RESULT_FAIL; // Нам не нужен второй экземпляр
    }
    else
    {
        // Инициализация для фоновых приложений
        app = (APP_HELLOWORLD_T*)APP_InitAppData( (void *)My_APP_HandleEvent, // Обработчик для фоновых приложений
                                                  sizeof(APP_HELLOWORLD_T), // Размер структуры приложения
                                                  reg_id,
                                                  0, 1,
                                                  1,
                                                  2, 0, 0 );
                                                  

        status = APP_Start( ev_st,
                            &app->apt,
                            HW_STATE_MAIN, // Начальное состояние
                            state_handling_table,
                            destroyApp,
                            app_name,
                            0 );

    }

    return RESULT_OK;
}


/* Функция выхода из приложения */
UINT32 destroyApp( EVENT_STACK_T *ev_st,  void *app )
{
    UINT32  status;

	if (destroy) {
		//CloseLog();
		
		/* Завершаем работу приложения */
	    status = APP_Exit( ev_st, app, NULL );
			
	    /* Выгружаем эльф */
	    LdrUnloadELF(&Lib); // &Lib указывает на начало эльфа
	}

    return status;
}

/* Обработчик входа в state */
UINT32 MainStateEnter( EVENT_STACK_T *ev_st,  void *app,  ENTER_STATE_TYPE_T type )
{
    return RESULT_OK;
}


void generate_filename(WCHAR *path);

	
/* Обработчик события, EV_KEY_PRESS в данном случае */
UINT32 HandleKeypress( EVENT_STACK_T *ev_st,  void *app )
{
	int i;
	EVENT_T     *event = AFW_GetEv(ev_st);
	char		key_str[8];

	PFprintf("KeyPress = %d\n", event->data.key_pressed);
	//strcpy(key_str, "Code keyPress=");
	//itoa(event->data.key_pressed, key_str+strlen(key_str), 10);
	//strcat(key_str, "\r\n");
	//SendLog(key_str);
	
	switch (event->data.key_pressed)
	{
		case KEY_STAR:
			destroy = true;
			return destroyApp(ev_st, app);
		case KEY_0: {
				UINT8 rom_data[HAPI_BATTERY_ROM_BYTE_SIZE];
				UINT64 unique;
				HAPI_BATTERY_ROM_T batt_type;
				FILE dump;
				UINT32 written;

				batt_type = HAPI_BATTERY_ROM_read(rom_data);
				HAPI_BATTERY_ROM_get_unique_id((UINT8 *)&unique);
				
				PFprintf("BATTERY UNIQUE = %llu\n", unique);
				PFprintf("RETURN = %d\n", batt_type);
				
				DL_FsDeleteFile( uri,  0 );
				dump = DL_FsOpenFile( uri,  FILE_WRITE_MODE,  0 );
				DL_FsWriteFile((void*) rom_data, HAPI_BATTERY_ROM_BYTE_SIZE, 1, dump, &written);
			}
			break;
		case KEY_8: {
			FILE ram_dump;
			UINT32 written;
			UINT16 *pixel_address;
			GRAPHIC_POINT_T pixel_coordinate;
			AHIDEVCONTEXT_T context = (AHIDEVCONTEXT_T)DAL_GetDeviceContext(0);
			
			pixel_coordinate.x = 0;
			pixel_coordinate.y = 1;
			
			pixel_address = DAL_GetDisplayPixelAddress(0, pixel_coordinate, DISPLAY_MAIN);
			
			/*
			x = 0;
			y = 0;
			DSB 1, 123336B4
			DSB 2, 123336B4
			PA 1, 00000000
			PA 2, 0000E59F
			
			x = 0;
			y = 1;
			DSB 1, 123336B8
			DSB 2, 123336B8
			PA 1, 00000000
			PA 2, 0000E59F
			
			DSB 1, 123336CC
			DSB 2, 123336CC
			PA 1, 00000000
			PA 2, 0000E59F
			PA 3, 0000F018
			*/

			PFprintf("DSB 1, %08X\n", display_source_buffer);
			PFprintf("DSB 2, %08X\n", &display_source_buffer[0]);
			PFprintf("PA 1, %08X\n", pixel_address);
			PFprintf("PA 2, %08X\n", *pixel_address);
			pixel_address++;
			PFprintf("PA 3, %08X\n", *pixel_address);
			
			DL_FsDeleteFile( L"/c/ram.dump",  0 );
			ram_dump = DL_FsOpenFile( L"/c/ram.dump",  FILE_WRITE_MODE,  0 );
			// DL_FsWriteFile((void *) 0x121FC008, 128*160*2, 1, ram_dump, &written);
			DL_FsWriteFile((void *) 0x12000000, 0x800000, 1, ram_dump, &written);
		}
		break;
		case KEY_5: {
			int ret;
			AHIDEVCONTEXT_T  context;
			AHISURFACE_T    surface;
			
			AHIBITMAP_T   bitmap;
			AHIRECT_T rect;
			AHIPOINT_T point;
			
			FILE dump;
			UINT32 written;
			GRAPHIC_POINT_T p;
			UINT16 *buffer;
			UINT16 *address;
			INT32 line, stroke, ppp, pixel;
			UINT16 bmp_pixel;
			
			UINT16 *image_buffer_lol;
			
			PFprintf("1\n");
			context = (AHIDEVCONTEXT_T)DAL_GetDeviceContext(0);
			PFprintf("2\n");
			surface = (AHISURFACE_T)DAL_GetDrawingSurface(0);
			PFprintf("3\n");
			
			image_buffer_lol = (UINT16 *) suAllocMem(128*160*2, NULL);
			
			PFprintf("4\n");
			
			bitmap.width  = 128;
            bitmap.height = 160;
            bitmap.stride = 128 * 2; // RGB565
            bitmap.format = AHIFMT_16BPP_565;
            bitmap.image = (void *) 0x12200254;

			PFprintf("5\n");
			
            rect.x1 = 0;
            rect.y1 = 0;
			rect.x2 = 0 + 128;
			rect.y2 = 0 + 160;
   
            point.x = 0;
            point.y = 0;
			
			PFprintf("6\n");
			
			
			ret = AhiSurfCopy(context, surface, &bitmap, &rect, &point, NULL, 1);
			
			PFprintf("7\n");
		
		
			
			//DL_FsDeleteFile( L"/a/scr.dump",  0 );
			
			//*buffer = 0xFFFF;
			
			PFprintf("8\n");

			generate_filename(path);
			dump = DL_FsOpenFile( path,  FILE_WRITE_MODE,  0 );
			
			PFprintf("9\n");
			
			DL_FsWriteFile(BMP_HEADER, BMP_HEADER_SIZE, 1, dump, &written);
			
			/*
			for (y = 160 - 1; y >= 0; --y)
				for (address = ((void *) 0x121FC008), x = 128 - 1; x >= 0; --x, address++) {
					bmp_pixel = ((*address) << 8) | ((*address) >> 8);
					image_buffer_lol[y + x * 128] = bmp_pixel;
				}
			*/
			
			/*
			line = 128;
			lol = 1;
			for (address = ((void *) 0x121FC008), pixels = 128*160 - 1; pixels >= 0; ++address, --pixels) {
				bmp_pixel = ((*address) << 8) | ((*address) >> 8);
				image_buffer_lol[pixels - line * lol] = bmp_pixel;
				line--;
				if (line == 0) {
					line = 128;
					lol++;
				}
			}
			*/

			/*

			for (c = 128, r = 0, address = ((void *) 0x121FC008), px_i = 128*160 - 1; px_i >= 0; ++address, --px_i, --c) {
				px_bmp = ((*address) << 8) | ((*address) >> 8);
				image_buffer_lol[128*160 - c - r * 128] = px_bmp;
				if (c == 0) {
					c = 128;
					r++;
				}
			}

			*/

			line = 128;
			stroke = 0;
			ppp = 0;
			for (pixel = 128*160 - 1, address = ((void *) 0x12200254); pixel >= 0; --pixel, ++address) {
				bmp_pixel = ((*address) << 8) | ((*address) >> 8);
				ppp = 128*160 - line - stroke * 128;

				image_buffer_lol[ppp] = bmp_pixel;
				line--;
				if (line == 0) {
					line = 128;
					stroke++;
				}
			}

			DL_FsWriteFile(image_buffer_lol, 128*160*2, 1, dump, &written);
			
			DL_FsCloseFile(dump);
			
			suFreeMem(image_buffer_lol);
			
			PFprintf("10 %d\n", ret);
		}	
		break;
	}
	
    return RESULT_OK;
}

void generate_filename(WCHAR *path) {
	char buffer[FILEURI_MAX_LEN];
	CLK_DATE_T date;
	CLK_TIME_T time;
	
	DL_ClkGetTime(&time);
	DL_ClkGetDate(&date);
	
	sprintf(buffer, "/c/mobile/picture/SCR_%02d%02d%04d_%02d%02d%02d.bmp", date.day, date.month, date.year, time.hour, time.minute, time.second);
	
	u_atou(buffer, path);
}

/*
UINT32 OpenLog(WCHAR *uri)
{
	DL_FsDeleteFile( uri,  0 );
	LogFile = DL_FsOpenFile( uri,  FILE_WRITE_MODE,  0 );
	return RESULT_OK;
}

UINT32 CloseLog()
{
	return DL_FsCloseFile(LogFile);
}

UINT32 SendLog(char *str)
{
  UINT32 written;
  
	DL_FsWriteFile( (void*) str,
                    strlen(str),
                    1, 
                    LogFile,
                    &written);
					
	return RESULT_OK;
}
*/





