/*
 * About:
 *
 * Author:
 *   EXL
 *
 * License:
 *   MIT
 *
 * Application type:
 *   GUI + UIS Canvas
 *
 * Additional information:
 *
 */

#include <loader.h>
#include <apps.h>
#include <uis.h>
#include <canvas.h>
#include <mem.h>
#include <utilities.h>
#include <dl.h>
#include <dl_keypad.h>

#define TIMER_FAST_TRIGGER_MS             (1)

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_MAX
} APP_STATE_T;

typedef enum {
	APP_TIMER_EXIT = 0x0001,
	APP_TIMER_EXIT_FAST
} APP_TIMER_T;

typedef enum {
	BENCHMARK_MAIN_SCREEN
} BENCHMARK_STATE_T;

typedef struct {
	APPLICATION_T app;

	GRAPHIC_REGION_T area;
} APP_INSTANCE_T;

#if defined(EP1)
UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code); /* ElfPack 1.x entry point. */
#elif defined(EP2)
ldrElf *_start(WCHAR *uri, WCHAR *arguments);                                /* ElfPack 2.x entry point. */
#elif defined(EPMCORE)
UINT32 ELF_Entry(ldrElf *elf, WCHAR *arguments);                             /* ElfPack M*CORE entry point. */
#endif

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);

static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 PaintAll(EVENT_STACK_T *ev_st, APPLICATION_T *app, BENCHMARK_STATE_T state);

static const char g_app_name[APP_NAME_LEN] = "Benchmark";

static const COLOR_T g_color_background   = { 0xBB, 0xAD, 0xA0, 0xFF };

#if defined(EP2)
static ldrElf g_app_elf;
#elif defined(EPMCORE)
static ldrElf *g_app_elf = NULL;
#endif

static EVENT_HANDLER_ENTRY_T g_state_any_hdls[] = {
	{ EV_REVOKE_TOKEN, APP_HandleUITokenRevoked },
	{ EV_TIMER_EXPIRED, HandleEventTimerExpired },
	{ STATE_HANDLERS_END, NULL }
};

static EVENT_HANDLER_ENTRY_T g_state_init_hdls[] = {
	{ EV_GRANT_TOKEN, APP_HandleUITokenGranted },
	{ STATE_HANDLERS_END, NULL }
};

static EVENT_HANDLER_ENTRY_T g_state_main_hdls[] = {
	{ EV_DONE, ApplicationStop },
	{ EV_DIALOG_DONE, ApplicationStop },
	{ EV_INK_KEY_PRESS, HandleEventKeyPress },
	{ EV_INK_KEY_RELEASE, HandleEventKeyRelease },
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_state_table_hdls[] = {
	{ APP_STATE_ANY, NULL, NULL, g_state_any_hdls },
	{ APP_STATE_INIT, NULL, NULL, g_state_init_hdls },
	{ APP_STATE_MAIN, HandleStateEnter, NULL, g_state_main_hdls }
};

#if defined(EP1)
UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code) {
	UINT32 status;
	UINT32 ev_code_base;

	ev_code_base = ev_code;

	status = APP_Register(&ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	LdrStartApp(ev_code_base);

	return status;
}
#elif defined(EP2)
ldrElf *_start(WCHAR *uri, WCHAR *arguments) {
	UINT32 status;
	UINT32 ev_code_base;
	UINT32 reserve;

	if (ldrIsLoaded(g_app_name)) {
		cprint("Benchmark: Error! Application has already been loaded!\n");
		return NULL;
	}

	status = RESULT_OK;
	ev_code_base = ldrRequestEventBase();
	reserve = ev_code_base + 1;
	reserve = ldrInitEventHandlersTbl(g_state_any_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_init_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_main_hdls, reserve);

	status |= APP_Register(&ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	status |= ldrSendEvent(ev_code_base);
	g_app_elf.name = (char *) g_app_name;

	return (status == RESULT_OK) ? &g_app_elf : NULL;
}
#elif defined(EPMCORE)
UINT32 ELF_Entry(ldrElf *elf, WCHAR *arguments) {
	UINT32 status;
	UINT32 reserve;

	status = RESULT_OK;
	g_app_elf = elf;
	g_app_elf->name = (char *) g_app_name;

	if (ldrIsLoaded(g_app_elf->name)) {
		PFprintf("%s: Application already loaded.\n", g_app_elf->name);
		return RESULT_FAIL;
	}

	reserve = g_app_elf->evbase + 1;
	reserve = ldrInitEventHandlersTbl(g_state_any_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_init_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_main_hdls, reserve);

	status |= APP_Register(&g_app_elf->evbase, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);
	if (status == RESULT_OK) {
		PFprintf("%s: Application has been registered successfully.\n", g_app_elf->name);

		status |= ldrSendEvent(g_app_elf->evbase);
	} else {
		PFprintf("%s: Cannot register application.\n", g_app_elf->name);
	}

	return status;
}
#endif

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_FAIL;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		app_instance = (APP_INSTANCE_T *) APP_InitAppData((void *) APP_HandleEvent, sizeof(APP_INSTANCE_T),
			reg_id, 0, 1, 1, 1, 1, 0);

		status = APP_Start(ev_st, &app_instance->app, APP_STATE_MAIN,
			g_state_table_hdls, ApplicationStop, g_app_name, 0);

#if defined(EP2)
		g_app_elf.app = (APPLICATION_T *) app_instance;
#elif defined(EPMCORE)
		g_app_elf->app = &app_instance->app;
#endif
	}

	return status;
}

static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	status = RESULT_OK;

	APP_ConsumeEv(ev_st, app);

	DeleteDialog(app);

	status |= APP_Exit(ev_st, app, 0);

#if defined(EP1)
	LdrUnloadELF(&Lib);
#elif defined(EP2)
	ldrUnloadElf();
#elif defined(EPMCORE)
	ldrUnloadElf(g_app_elf);
#endif

	return status;
}

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state) {
	APP_INSTANCE_T *app_instance;
	SU_PORT_T port;
	CONTENT_T content;
	UIS_DIALOG_T dialog;
	APP_STATE_T app_state;
	DRAWING_BUFFER_T buffer;
	GRAPHIC_POINT_T point;

	if (state != ENTER_STATE_ENTER) {
		if (app->state != APP_STATE_MAIN) {
			return RESULT_OK;
		}
	}

	app_instance = (APP_INSTANCE_T *) app;

	DeleteDialog(app);

	port = app->port;
	app_state = app->state;
	dialog = DialogType_None;

	memclr(&content, sizeof(CONTENT_T));

	switch (app_state) {
		case APP_STATE_MAIN:
#if defined(EPMCORE)
			UIS_CanvasGetDisplaySize(&point);
#else
			point = UIS_CanvasGetDisplaySize();
#endif
			buffer.w = point.x + 1;
			buffer.h = point.y + 1;
			buffer.buf = NULL;

			app_instance->area.ulc.x = 0;
			app_instance->area.ulc.y = 0;
			app_instance->area.lrc.x = buffer.w;
			app_instance->area.lrc.y = buffer.h;

			dialog = UIS_CreateColorCanvas(&port, &buffer, TRUE);
			break;
		default:
			break;
	}

	if (dialog == DialogType_None) {
		return RESULT_FAIL;
	}

	app->dialog = dialog;

	switch (app_state) {
		case APP_STATE_MAIN:
			PaintAll(ev_st, app, BENCHMARK_MAIN_SCREEN);
			break;
		default:
			break;
	}

	return RESULT_OK;
}

static UINT32 DeleteDialog(APPLICATION_T *app) {
	if (app->dialog != DialogType_None) {
		UIS_Delete(app->dialog);
		app->dialog = DialogType_None;
		return RESULT_OK;
	}

	return RESULT_FAIL;
}

typedef UINT8 HAPI_CLOCK_RATES_T;

typedef struct
{
    UINT16 reserved1;
    UINT8 load;
    UINT8 reserved2;
} REF_DPLL_CTRL_BYTE_ACCESS_T;

typedef union
{
    UINT32 word_access;
    REF_DPLL_CTRL_BYTE_ACCESS_T byte_access;
} REF_DPLL_CTRL_T;

typedef struct
{
    UINT32 usb_dpll_ctrl;
    UINT32 usb_dpll_op;
    UINT32 usb_dpll_mfn;
    UINT32 usb_dpll_mfd;
    REF_DPLL_CTRL_T ref_dpll_ctrl;
    UINT32 ref_dpll_op;
    UINT32 ref_dpll_mfn;
    UINT32 ref_dpll_mfd;
    UINT32 div_factor;
    UINT32 clk_sel;
    UINT32 clk_monitor;
    UINT32 low_pwr_mode;
//#if (MAKE_HDW_BB_IC == MAKE_HDW_NEPTUNE_LT)
//    UINT32 dsp_clk_ctrl;
//#endif
//#if (MAKE_HDW_BB_IC_LTS_SUBSET >= MAKE_HDW_BB_IC_LTS_SUBSET_LTE)
    UINT32 ref_mfn_minus;
    UINT32 ref_mfn_plus;
    UINT32 mfn_tog_cnt;
    UINT32 desense_stat;
//#endif
} HAPI_CLOCK_REG_T;

typedef struct
{
    UINT8 mfi;
    UINT8 pdf;
    UINT32 mfd;
    UINT32 mfn;
} DPLL_REG_VALUE_TBL_T;

enum
{
    HAPI_CLOCK_RATES_INVALID = 0,
    HAPI_CLOCK_RATES_13_MHZ,
    HAPI_CLOCK_RATES_16_8_MHZ,
    HAPI_CLOCK_RATES_19_44_MHZ,
    HAPI_CLOCK_RATES_26_MHZ,
    HAPI_CLOCK_RATES_39_MHZ,
    HAPI_CLOCK_RATES_40_MHZ,
    HAPI_CLOCK_RATES_48_MHZ,
    HAPI_CLOCK_RATES_52_MHZ,
    HAPI_CLOCK_RATES_104_MHZ,
    HAPI_CLOCK_RATES_156_MHZ,
    HAPI_CLOCK_RATES_208_MHZ,
    HAPI_CLOCK_RATES_260_MHZ,
    HAPI_DPLL_RATES_MAX
};

#define DPLL_LOCK_FLAG_MASK    0x00000001   /* LRF */
#define DPLL_BIN_RATE_MOD_MASK 0x00000002   /* BRMO */
#define DPLL_LOCK_MODE_MASK    0x00000004   /* PLM */
#define DPLL_ENABLE_MASK       0x00000020   /* PEN */
#define DPLL_SW_RESET_MASK     0x00000040   /* PRE */
#define DPLL_LOAD_REQ_MASK     0x00000100   /* LDREQ */
#define DPLL_LOAD_REQ_BYTE_MASK      0x01   /* LDREQ */
#define MCU_DPLL_DIV_MASK      0x0000000f   /* MCD */
#define USB_DPLL_DIV_MASK      0x000000f0   /* UDD */
#define REF_DPLL_DIV_MASK      0x00000f00   /* RDD */
#define MCU_CLK_SEL_MASK       0x00000003   /* MCS */
#define DIV_CLK_SEL_MASK       0x0000000c   /* DCS */
#define REF_SEL_MASK           0x00000010   /* REF_SEL */
#define CODEC_CLK_SEL_MASK     0x00000020   /* CODS */

/*To cover LT product from failure */
#define CKOS_SHIFT             0x00000000
#define CKOS_MASK              0x00000000


//#if (MAKE_HDW_BB_IC == MAKE_HDW_NEPTUNE_LTS)
//# define QCLK_BYP_MASK         0x00000040   /* QCLK_BYP */
//# define DBG_EN_MASK           0x00000080   /* DBG_EN */
//#endif
//#if (MAKE_HDW_BB_IC == MAKE_HDW_NEPTUNE_LT)
//# define DSP_CLK_GENS_MASK     0x00000040   /* CLKGENS */
//#endif
#define USB_BYP_MASK           0x00000200   /* USB_BYP */
#define DPLL_BYP_MASK          0x00000800   /* DPLL_BYP */
#define DSM_STANDBY_CTL_MASK   0x00000020   /* STBY_CTL */
#define DIS_GLLITCHLESS_MASK   0x00001000   /* DIS_GL */
#define CKOHS_MASK             0x00007000   /* CKOHS */
#define PWR_DWN_AMP_MASK       0x00000004   /* PDN_AMP */
#define L1T_MCEN_MASK          0x00080000   /* L1T_MCEN */
//#if (MAKE_HDW_BB_IC == MAKE_HDW_NEPTUNE_LTS)
//# undef  CKOS_MASK
//# define CKOS_MASK             0x00000700   /* CKOS */
//# define CKIL_FLAG_MASK        0x00000040   /* NO_CKIL_CLK */
//# define CKIL_MON_MASK         0x00000080   /* CKIL_MON_EN */
//#endif
#define LOW_PWR_MODE_MASK      0x00000003   /* LPMD */
//#if (MAKE_HDW_BB_IC == MAKE_HDW_NEPTUNE_LT)
//# define DSP_DIV_FACTOR_MASK   0x00000007   /* DF */
//# define DSP_STOP_MASK         0x00000010   /* PSTP */
//# define DSP_DPLL_SEL_MASK     0x00000800   /* PCE */
//#endif

//#if ((MAKE_HDW_BB_IC == MAKE_HDW_NEPTUNE_LTS) &&
//     (MAKE_HDW_BB_IC_LTS_SUBSET >= MAKE_HDW_BB_IC_LTS_SUBSET_LTE))
//#endif
# define TOG_CNT_MASK       0x0000FFFF   /* TOG_CNT */
# define TOG_EN_MASK        0x00010000   /* TOG_EN */
//#endif

#define MCU_DPLL_DIV_SHIFT     0
#define USB_DPLL_DIV_SHIFT     4
#define REF_DPLL_DIV_SHIFT     8
#define MCU_CLK_SEL_SHIFT      0
#define LOW_PWR_MODE_SHIFT     0
#define DIV_CLK_SEL_SHIFT      2
#define MCU_CLK_SEL_SHIFT      0
#define DIV_CLK_SEL_SHIFT      2
#define DPLL_BYP_SHIFT         11
//#if (MAKE_HDW_BB_IC == MAKE_HDW_NEPTUNE_LTS)
//# undef CKOS_SHIFT
//# define CKOS_SHIFT            8
//# define CKO_PAT_REF           5
//#endif
#define MCU_CLK_IN_SEL         1
#define CCM_REF_DPLL_SEL       1
#define DPLL_BYP               1


#define DPLL_DIVIDE_BY_1       0xf
#define DPLL_DIVIDE_BY_2       0x0
#define DPLL_DIVIDE_BY_4       0x2
#define DPLL_DIVIDE_BY_6       0x4
#define DPLL_DIVIDE_BY_8       0x5
//#if (MAKE_HDW_BB_IC == MAKE_HDW_NEPTUNE_LTS)
# define DPLL_DIVIDE_BY_5       0x3
# define DPLL_DIVIDE_BY_10      0x6
//#endif

#define MFI_SHIFT 4
#define PDF_SHIFT 0
#define MFD_SHIFT 0
#define MFN_SHIFT 0

/* Invalid entry will cause unity multiplication factor */
#define MFI_INVALID (5 << MFI_SHIFT)
#define PDF_INVALID (9 << PDF_SHIFT)
#define MFD_INVALID (0 << MFD_SHIFT)
#define MFN_INVALID (0 << MFN_SHIFT)


/*************************** 26 MHZ Ref Freq DPLL Values (REF DPLL) ***************************/

#define REF_26_MFI_13MHZ (5  << MFI_SHIFT)
#define REF_26_PDF_13MHZ (19 << PDF_SHIFT)
#define REF_26_MFD_13MHZ (2096774  << MFD_SHIFT)
#define REF_26_MFN_13MHZ (0  << MFN_SHIFT)

#define REF_26_MFI_16_8MHZ (5  << MFI_SHIFT)
#define REF_26_PDF_16_8MHZ (19 << PDF_SHIFT)
#define REF_26_MFD_16_8MHZ (12 << MFD_SHIFT)
#define REF_26_MFN_16_8MHZ (19 << MFN_SHIFT)

#define REF_26_MFI_19_44MHZ (13  << MFI_SHIFT)
#define REF_26_PDF_19_44MHZ (51  << PDF_SHIFT)
#define REF_26_MFD_19_44MHZ (99  << MFD_SHIFT)
#define REF_26_MFN_19_44MHZ (644 << MFN_SHIFT)

#define REF_26_MFI_26MHZ (5  << MFI_SHIFT)
#define REF_26_PDF_26MHZ (9  << PDF_SHIFT)
#define REF_26_MFD_26MHZ (2096774  << MFD_SHIFT)
#define REF_26_MFN_26MHZ (0  << MFN_SHIFT)

#define REF_26_MFI_39MHZ (6  << MFI_SHIFT)
#define REF_26_PDF_39MHZ (7   << PDF_SHIFT)
#define REF_26_MFD_39MHZ (2096774   << MFD_SHIFT)
#define REF_26_MFN_39MHZ (0   << MFN_SHIFT)

#define REF_26_MFI_40MHZ (6     << MFI_SHIFT)
#define REF_26_PDF_40MHZ (7     << PDF_SHIFT)
#define REF_26_MFD_40MHZ (1014  << MFD_SHIFT)
#define REF_26_MFN_40MHZ (156   << MFN_SHIFT)

#define REF_26_MFI_48MHZ (7   << MFI_SHIFT)
#define REF_26_PDF_48MHZ (7   << PDF_SHIFT)
#define REF_26_MFD_48MHZ (506 << MFD_SHIFT)
#define REF_26_MFN_48MHZ (195 << MFN_SHIFT)

#define REF_26_MFI_52MHZ (5  << MFI_SHIFT)
#define REF_26_PDF_52MHZ (4  << PDF_SHIFT)
#define REF_26_MFD_52MHZ (2096774  << MFD_SHIFT)
#define REF_26_MFN_52MHZ (0  << MFN_SHIFT)

#define REF_26_MFI_104MHZ (8  << MFI_SHIFT)
#define REF_26_PDF_104MHZ (3  << PDF_SHIFT)
#define REF_26_MFD_104MHZ (2096774  << MFD_SHIFT)
#define REF_26_MFN_104MHZ (0  << MFN_SHIFT)

#define REF_26_MFI_156MHZ (12  << MFI_SHIFT)
#define REF_26_PDF_156MHZ (3  << PDF_SHIFT)
#define REF_26_MFD_156MHZ (2096774  << MFD_SHIFT)
#define REF_26_MFN_156MHZ (0  << MFN_SHIFT)

#define REF_26_MFI_208MHZ (12  << MFI_SHIFT)
#define REF_26_PDF_208MHZ (2  << PDF_SHIFT)
#define REF_26_MFD_208MHZ (8388606  << MFD_SHIFT) /* MFN will represent +-0.129Hz step at PAT REF 13MHz */
#define REF_26_MFN_208MHZ (0  << MFN_SHIFT)

//#if (MAKE_HDW_EZX == TRUE)
//# define REF_26_MFI_260MHZ (10  << MFI_SHIFT)
//# define REF_26_PDF_260MHZ (1  << PDF_SHIFT)
//# define REF_26_MFD_260MHZ (2096774  << MFD_SHIFT) /* MFN will represent +-0.775Hz step at PAT REF 13MHz */
//# define REF_26_MFN_260MHZ (0  << MFN_SHIFT)
//#else
# define REF_26_MFI_260MHZ (15  << MFI_SHIFT)
# define REF_26_PDF_260MHZ (2  << PDF_SHIFT)
# define REF_26_MFD_260MHZ (8388606  << MFD_SHIFT) /* MFN will represent +-0.103Hz step at PAT REF 13MHz */
# define REF_26_MFN_260MHZ (0  << MFN_SHIFT)
//#endif

/*
 * Define the values used in HAPI_CLOCK_mcu_set for the different chipsets.
 */

# define DPLL_CLOCK_RATE_26_MHZ HAPI_CLOCK_RATES_260_MHZ
# define RDD_DIVIDE_26_MHZ      DPLL_DIVIDE_BY_10
# define MCD_DIVIDE_26_MHZ      DPLL_DIVIDE_BY_10

# define DPLL_CLOCK_RATE_39_MHZ HAPI_CLOCK_RATES_156_MHZ
# define RDD_DIVIDE_39_MHZ      DPLL_DIVIDE_BY_6
# define MCD_DIVIDE_39_MHZ      DPLL_DIVIDE_BY_4

//#if (MAKE_HDW_BB_IC_LTS_SUBSET >= MAKE_HDW_BB_IC_LTS_SUBSET_LTE)
# define DPLL_CLOCK_RATE_52_MHZ HAPI_CLOCK_RATES_260_MHZ
# define RDD_DIVIDE_52_MHZ      DPLL_DIVIDE_BY_10
# define MCD_DIVIDE_52_MHZ      DPLL_DIVIDE_BY_5
//#else
//# define DPLL_CLOCK_RATE_52_MHZ HAPI_CLOCK_RATES_208_MHZ
//# define RDD_DIVIDE_52_MHZ      DPLL_DIVIDE_BY_8
//# define MCD_DIVIDE_52_MHZ      DPLL_DIVIDE_BY_4
//#endif

#define HAPI_CLOCK_IS_REF_DPLL_LOCK  ((hapi_clock_reg->ref_dpll_ctrl.word_access & DPLL_LOCK_FLAG_MASK) == DPLL_LOCK_FLAG_MASK)

static const DPLL_REG_VALUE_TBL_T REF_26_DPLL_REG_VALUES[HAPI_DPLL_RATES_MAX] =
{
    /* Invalid */
    {
        MFI_INVALID,
        PDF_INVALID,
        MFD_INVALID,
        MFN_INVALID
    },
    /* 13 MHz */
    {
        REF_26_MFI_13MHZ,
        REF_26_PDF_13MHZ,
        REF_26_MFD_13MHZ,
        REF_26_MFN_13MHZ
    },
    /* 16.8 MHz */
    {
        REF_26_MFI_16_8MHZ,
        REF_26_PDF_16_8MHZ,
        REF_26_MFD_16_8MHZ,
        REF_26_MFN_16_8MHZ
    },
    /* 19.44 MHz */
    {
        REF_26_MFI_19_44MHZ,
        REF_26_PDF_19_44MHZ,
        REF_26_MFD_19_44MHZ,
        REF_26_MFN_19_44MHZ
    },
    /* 26 MHz */
    {
        REF_26_MFI_26MHZ,
        REF_26_PDF_26MHZ,
        REF_26_MFD_26MHZ,
        REF_26_MFN_26MHZ
    },
    /* 39 MHz */
    {
        REF_26_MFI_39MHZ,
        REF_26_PDF_39MHZ,
        REF_26_MFD_39MHZ,
        REF_26_MFN_39MHZ
    },
    /* 40 MHz */
    {
        REF_26_MFI_40MHZ,
        REF_26_PDF_40MHZ,
        REF_26_MFD_40MHZ,
        REF_26_MFN_40MHZ
    },
    /* 48 MHz */
    {
        REF_26_MFI_48MHZ,
        REF_26_PDF_48MHZ,
        REF_26_MFD_48MHZ,
        REF_26_MFN_48MHZ
    },
    /* 52 MHz */
    {
        REF_26_MFI_52MHZ,
        REF_26_PDF_52MHZ,
        REF_26_MFD_52MHZ,
        REF_26_MFN_52MHZ
    },
         /* 104 MHz */
    {
        REF_26_MFI_104MHZ,
        REF_26_PDF_104MHZ,
        REF_26_MFD_104MHZ,
        REF_26_MFN_104MHZ
    },
         /* 156 MHz */
    {
        REF_26_MFI_156MHZ,
        REF_26_PDF_156MHZ,
        REF_26_MFD_156MHZ,
        REF_26_MFN_156MHZ
    },
         /* 208 MHz */
    {
        REF_26_MFI_208MHZ,
        REF_26_PDF_208MHZ,
        REF_26_MFD_208MHZ,
        REF_26_MFN_208MHZ
    },
         /* 260 MHz */
    {
        REF_26_MFI_260MHZ,
        REF_26_PDF_260MHZ,
        REF_26_MFD_260MHZ,
        REF_26_MFN_260MHZ
    }
};

/* Used to configure the CMON register */
#define   HAPI_CLOCK_CKOS_DISABLE    0x0800
#define   CKOHS_SHIFT                12
#define   PDN_CMON_DISABLE           0x0000 /* disable the Clock MONitor Power DowN*/
#define   PDN_CMON_MASK              0x0008 /* mask to access to PDN_CMON*/

// 13000000 hz  - 1410 - 83  - 1
// 64999996 hz  - 1410 - 242 - 2
// 64999993 hz  - 1154 - 242 - 3
// 25999996 hz  - 1670 - 242 - 4
// 38999965 hz  - 1154 - 195 - 5
// 51999983 hz  - 1667 - 242 - 8
// 64999987 hz  - 1666 - 242 - 9
// 86666634 hz  - 1153 - 242 - 0
// 129999963 hz -  640 - 242 - 7
// 12999972 hz  - 1667 - 103 - 6

void HAPI_CLOCK_mcu_setaa(HAPI_CLOCK_RATES_T clock_rate)
{
    HAPI_CLOCK_RATES_T dpll_clock_rate;
    /* Create a pointer to the constant DPLL register value tables */
    const DPLL_REG_VALUE_TBL_T * dpll_reg_value_tbl_ptr;

	UINT8 *hapi_clock_rate_mcu = (void *) 0x3FC3600;
	HAPI_CLOCK_REG_T *hapi_clock_reg = (void *) 0x24845000;

    switch(clock_rate)
    {
        case HAPI_CLOCK_RATES_13_MHZ:
            /* Set for wait states and waste time MCU_CLK = 13MHZ*/
            *hapi_clock_rate_mcu = HAPI_CLOCK_RATES_13_MHZ;
            /* Select PAT_REF to MCU_CLK */
            hapi_clock_reg->clk_sel &= ~MCU_CLK_SEL_MASK;
            /* Set PAT_REF Uncorrected 26 MHZ */
            hapi_clock_reg->clk_sel &= ~REF_SEL_MASK;
            /* Set CODEC to Uncorrected 26 MHZ */
            hapi_clock_reg->clk_sel |= CODEC_CLK_SEL_MASK;
            break;

        case HAPI_CLOCK_RATES_26_MHZ:
//            *hapi_clock_rate_mcu = HAPI_CLOCK_RATES_26_MHZ;
            dpll_clock_rate = DPLL_CLOCK_RATE_26_MHZ;
            hapi_clock_reg->div_factor =
                (hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | (RDD_DIVIDE_26_MHZ << REF_DPLL_DIV_SHIFT);
            hapi_clock_reg->div_factor =
                (hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | (MCD_DIVIDE_26_MHZ << MCU_DPLL_DIV_SHIFT);
            break;

        case HAPI_CLOCK_RATES_39_MHZ:
            *hapi_clock_rate_mcu = HAPI_CLOCK_RATES_39_MHZ;
            dpll_clock_rate = DPLL_CLOCK_RATE_39_MHZ;
            hapi_clock_reg->div_factor =
                (hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | (RDD_DIVIDE_39_MHZ << REF_DPLL_DIV_SHIFT);
            hapi_clock_reg->div_factor =
                 (hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | (MCD_DIVIDE_39_MHZ << MCU_DPLL_DIV_SHIFT);
            break;

        case HAPI_CLOCK_RATES_52_MHZ:
            *hapi_clock_rate_mcu = HAPI_CLOCK_RATES_52_MHZ;
            dpll_clock_rate = DPLL_CLOCK_RATE_52_MHZ;
            hapi_clock_reg->div_factor =
                (hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | (RDD_DIVIDE_52_MHZ << REF_DPLL_DIV_SHIFT);
            hapi_clock_reg->div_factor =
                (hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | (MCD_DIVIDE_52_MHZ << MCU_DPLL_DIV_SHIFT);
            break;

	case 2:
		*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_52_MHZ;
		dpll_clock_rate = DPLL_CLOCK_RATE_52_MHZ;
		hapi_clock_reg->div_factor =
			(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | 0x500; //(RDD_DIVIDE_39_MHZ << REF_DPLL_DIV_SHIFT);
		hapi_clock_reg->div_factor =
			 (hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | 0x02; //(MCD_DIVIDE_39_MHZ << MCU_DPLL_DIV_SHIFT);
		break;

	case 3:
		*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_52_MHZ;
		dpll_clock_rate = DPLL_CLOCK_RATE_52_MHZ;
		hapi_clock_reg->div_factor =
			(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | 0x400; //(RDD_DIVIDE_39_MHZ << REF_DPLL_DIV_SHIFT);
		hapi_clock_reg->div_factor =
			 (hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | 0x02; //(MCD_DIVIDE_39_MHZ << MCU_DPLL_DIV_SHIFT);
		break;

	case 9:
		*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_52_MHZ;
		dpll_clock_rate = DPLL_CLOCK_RATE_52_MHZ;
		hapi_clock_reg->div_factor =
			(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | 0x600; //(RDD_DIVIDE_39_MHZ << REF_DPLL_DIV_SHIFT);
		hapi_clock_reg->div_factor =
			 (hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | 0x02; //(MCD_DIVIDE_39_MHZ << MCU_DPLL_DIV_SHIFT);
		break;

	case 0:
		*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_52_MHZ;
		dpll_clock_rate = DPLL_CLOCK_RATE_52_MHZ;
		hapi_clock_reg->div_factor =
			(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | 0x400; //(RDD_DIVIDE_39_MHZ << REF_DPLL_DIV_SHIFT);
		hapi_clock_reg->div_factor =
			 (hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | 0x01; //(MCD_DIVIDE_39_MHZ << MCU_DPLL_DIV_SHIFT);
		break;

	case 7:
		*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_52_MHZ;
		dpll_clock_rate = DPLL_CLOCK_RATE_52_MHZ;
		hapi_clock_reg->div_factor =
			(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | 0x200; //(RDD_DIVIDE_39_MHZ << REF_DPLL_DIV_SHIFT);
		hapi_clock_reg->div_factor =
			 (hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | 0x00; //(MCD_DIVIDE_39_MHZ << MCU_DPLL_DIV_SHIFT);
		break;

        default:
//            suPanic(HAPI_PANIC_INVALID_CLK_RATE, 0);
            break;
    }
//    if (clock_rate != HAPI_CLOCK_RATES_13_MHZ)
    {
        /* Use fref 26MHZ constants table */
        dpll_reg_value_tbl_ptr = REF_26_DPLL_REG_VALUES;

        /* Configure the DPLL to the desired dpll clock rate */
        hapi_clock_reg->ref_dpll_op =
            dpll_reg_value_tbl_ptr[dpll_clock_rate].mfi | dpll_reg_value_tbl_ptr[dpll_clock_rate].pdf;
        hapi_clock_reg->ref_dpll_mfd = dpll_reg_value_tbl_ptr[dpll_clock_rate].mfd;
        hapi_clock_reg->ref_dpll_mfn = dpll_reg_value_tbl_ptr[dpll_clock_rate].mfn;


        hapi_clock_reg->clk_monitor &= (~PWR_DWN_AMP_MASK);
        hapi_clock_reg->ref_dpll_ctrl.word_access |= (DPLL_ENABLE_MASK | DPLL_BIN_RATE_MOD_MASK);

        /* Wait for the DPLL to stabilize */
//        while (HAPI_CLOCK_IS_REF_DPLL_LOCK == FALSE);

        /* Select CCM_REF_DPLL_OUT to MCU_CLK_IN */
        hapi_clock_reg->clk_sel =
            (hapi_clock_reg->clk_sel & ~DIV_CLK_SEL_MASK) | (CCM_REF_DPLL_SEL << DIV_CLK_SEL_SHIFT);
        /* Select MCU_CLK_IN to MCU_CLK */
        hapi_clock_reg->clk_sel =
            (hapi_clock_reg->clk_sel & ~MCU_CLK_SEL_MASK) | (MCU_CLK_IN_SEL << MCU_CLK_SEL_SHIFT);
        /* Set PAT_REF to Corrected/Uncorrected Depend on DSM */
        hapi_clock_reg->clk_sel |= REF_SEL_MASK;
        /* Set CODEC to corrected 26 MHZ */
        hapi_clock_reg->clk_sel &= ~CODEC_CLK_SEL_MASK;
    }
}

static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	EVENT_T *event;
	int base, step;

	/*
	 * Motorola V235 signals finding, thanks to @Indepth6
	 *
	 * RANGE phase 1:
	 *     600...824, step is 25.
	 * Found vibromotor signal on Key 5:
	 *     700...724, vibromotor signal is here.
	 * RANGE phase 2:
	 *     700...726, step is 3.
	 * Found vibromotor signal on Key 8:
	 *     721...723, vibromotor signal is here.
	 *
	 *     700-702 - 1
	 *     703-705 - 2
	 *     706-708 - 3
	 *     709-711 - 4
	 *     712-714 - 5
	 *     715-717 - 6
	 *     718-720 - 7
	 *     721-723 - 8
	 *     724-726 - 9
	 *
	 * Found vibromotor signal on 723.
	 *
	 * Voltage vibromotor signal on 690?
	 *     735 - 702 = 33
	 *     723 - 33 = 690
	 */

	/*
	 * Motorola V300 signals finding.
	 *
	 * Vibromotor signal is 38.
	 * Voltage vibromotor signal is 239?
	 */

//	base = 600;
//	step = 25;
	base = 700;
	step = 3;

	status = RESULT_OK;
	event = AFW_GetEv(ev_st);

	APP_ConsumeEv(ev_st, app);

	switch (event->data.key_pressed) {
		case KEY_1:
			HAPI_CLOCK_mcu_setaa(1);
			break;
		case KEY_2:
			HAPI_CLOCK_mcu_setaa(2);
			break;
		case KEY_3:
			HAPI_CLOCK_mcu_setaa(3);
			break;
		case KEY_4:
			HAPI_CLOCK_mcu_setaa(4);
			break;
		case KEY_5:
			HAPI_CLOCK_mcu_setaa(5);
			break;
		case KEY_6:
			HAPI_CLOCK_mcu_setaa(6);
			break;
		case KEY_7:
			HAPI_CLOCK_mcu_setaa(7);
			break;
		case KEY_8:
			HAPI_CLOCK_mcu_setaa(8);
			break;
		case KEY_9:
			HAPI_CLOCK_mcu_setaa(9);
			break;
		case KEY_0:
			HAPI_CLOCK_mcu_setaa(0);
			break;
		default:
			break;
	}

	PaintAll(ev_st, app, BENCHMARK_MAIN_SCREEN);

	return status;
}

static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	EVENT_T *event;

	status = RESULT_OK;
	event = AFW_GetEv(ev_st);

	APP_ConsumeEv(ev_st, app);

	switch (event->data.key_pressed) {
		case KEY_RED:
		case KEY_SOFT_LEFT:
//		case KEY_0:
			status |= APP_UtilStartTimer(TIMER_FAST_TRIGGER_MS, APP_TIMER_EXIT, app);
			break;
		default:
			break;
	}

	return status;
}

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	EVENT_T *event;
	APP_TIMER_T timer_id;

	event = AFW_GetEv(ev_st);
	timer_id = ((DL_TIMER_DATA_T *) event->attachment)->ID;

	switch (timer_id) {
		case APP_TIMER_EXIT:
			/* No break here. */
		case APP_TIMER_EXIT_FAST:
			/* Play an exit sound using quiet speaker. */
			DL_AudPlayTone(0x00,  0xFF);
			return ApplicationStop(ev_st, app);
			break;
		default:
			break;
	}

	return RESULT_OK;
}

static UINT32 PaintAll(EVENT_STACK_T *ev_st, APPLICATION_T *app, BENCHMARK_STATE_T state) {
	APP_INSTANCE_T *app_instance;
	GRAPHIC_POINT_T point;
	COLOR_T color;
	WCHAR *text;
	HAPI_CLOCK_REG_T *hapi_clock_reg = (void *) 0x24845000;
	WCHAR num[16];
	app_instance = (APP_INSTANCE_T *) app;



	switch (state) {
		case BENCHMARK_MAIN_SCREEN:
			UIS_CanvasSetFillColor(g_color_background);
			UIS_CanvasFillRect(app_instance->area, app->dialog);
			color.red = 0x00;
			color.green = 0x00;
			color.blue = 0x00;
			color.transparent = 0x00;
			UIS_CanvasSetForegroundColor(color);
			color.transparent = 0xFF;
			UIS_CanvasSetBackgroundColor(color);
			point.x = 10;
			point.y = 10;
			text = L"Benchmark P2K";
			UIS_CanvasDrawColorText(text, 0, u_strlen(text), point, 0, app->dialog);
			point.x = 10;
			point.y = 35;
			text = L"Press some button!";
			UIS_CanvasDrawColorText(text, 0, u_strlen(text), point, 0, app->dialog);

			u_ltou(hapi_clock_reg->div_factor, num);
			point.x = 10;
			point.y = 50;
			UIS_CanvasDrawColorText(num, 0, u_strlen(num), point, 0, app->dialog);

			u_ltou(hapi_clock_reg->ref_dpll_op, num);
			point.x = 10;
			point.y = 75;
			UIS_CanvasDrawColorText(num, 0, u_strlen(num), point, 0, app->dialog);

			break;
		default:
			break;
	}
	return RESULT_OK;
}


