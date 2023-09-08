#include <loader.h>
#include <utilities.h>

#include "Overclock.h"

#define HAPI_CLOCK_REG_ADDRESS    0x24845000

#if defined(FTR_L7E)
#define HAPI_CLOCK_RATE_ADDRESS   0x03FEFFF0
#else
#define HAPI_CLOCK_RATE_ADDRESS   0x03FC3600
#endif

typedef enum {
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
} HAPI_CLOCK_RATES_T;

#define PWR_DWN_AMP_MASK          0x00000004   /* PDN_AMP */
#define DPLL_ENABLE_MASK          0x00000020   /* PEN */
#define DPLL_BIN_RATE_MOD_MASK    0x00000002   /* BRMO */
#define DPLL_LOCK_FLAG_MASK       0x00000001   /* LRF */
#define DIV_CLK_SEL_MASK          0x0000000c   /* DCS */
#define MCU_CLK_SEL_MASK          0x00000003   /* MCS */
#define REF_SEL_MASK              0x00000010   /* REF_SEL */
#define CODEC_CLK_SEL_MASK        0x00000020   /* CODS */
#define REF_DPLL_DIV_MASK         0x00000f00   /* RDD */
#define MCU_DPLL_DIV_MASK         0x0000000f   /* MCD */

#define MCU_DPLL_DIV_SHIFT        0
#define USB_DPLL_DIV_SHIFT        4
#define REF_DPLL_DIV_SHIFT        8
#define CCM_REF_DPLL_SEL          1
#define DIV_CLK_SEL_SHIFT         2
#define MCU_CLK_IN_SEL            1
#define MCU_CLK_SEL_SHIFT         0

#define DPLL_DIVIDE_BY_1          0xf
#define DPLL_DIVIDE_BY_2          0x0
#define DPLL_DIVIDE_BY_3          0x1
#define DPLL_DIVIDE_BY_4          0x2
#define DPLL_DIVIDE_BY_6          0x4
#define DPLL_DIVIDE_BY_8          0x5
#define DPLL_DIVIDE_BY_5          0x3
#define DPLL_DIVIDE_BY_10         0x6

#define MFI_SHIFT                 4
#define PDF_SHIFT                 0
#define MFD_SHIFT                 0
#define MFN_SHIFT                 0

#define MFI_INVALID               (5 << MFI_SHIFT)
#define PDF_INVALID               (9 << PDF_SHIFT)
#define MFD_INVALID               (0 << MFD_SHIFT)
#define MFN_INVALID               (0 << MFN_SHIFT)

#define REF_26_MFI_13MHZ          (5  << MFI_SHIFT)
#define REF_26_PDF_13MHZ          (19 << PDF_SHIFT)
#define REF_26_MFD_13MHZ          (2096774  << MFD_SHIFT)
#define REF_26_MFN_13MHZ          (0  << MFN_SHIFT)
#define REF_26_MFI_16_8MHZ        (5  << MFI_SHIFT)
#define REF_26_PDF_16_8MHZ        (19 << PDF_SHIFT)
#define REF_26_MFD_16_8MHZ        (12 << MFD_SHIFT)
#define REF_26_MFN_16_8MHZ        (19 << MFN_SHIFT)
#define REF_26_MFI_19_44MHZ       (13  << MFI_SHIFT)
#define REF_26_PDF_19_44MHZ       (51  << PDF_SHIFT)
#define REF_26_MFD_19_44MHZ       (99  << MFD_SHIFT)
#define REF_26_MFN_19_44MHZ       (644 << MFN_SHIFT)
#define REF_26_MFI_26MHZ          (5  << MFI_SHIFT)
#define REF_26_PDF_26MHZ          (9  << PDF_SHIFT)
#define REF_26_MFD_26MHZ          (2096774  << MFD_SHIFT)
#define REF_26_MFN_26MHZ          (0  << MFN_SHIFT)
#define REF_26_MFI_39MHZ          (6  << MFI_SHIFT)
#define REF_26_PDF_39MHZ          (7   << PDF_SHIFT)
#define REF_26_MFD_39MHZ          (2096774   << MFD_SHIFT)
#define REF_26_MFN_39MHZ          (0   << MFN_SHIFT)
#define REF_26_MFI_40MHZ          (6     << MFI_SHIFT)
#define REF_26_PDF_40MHZ          (7     << PDF_SHIFT)
#define REF_26_MFD_40MHZ          (1014  << MFD_SHIFT)
#define REF_26_MFN_40MHZ          (156   << MFN_SHIFT)
#define REF_26_MFI_48MHZ          (7   << MFI_SHIFT)
#define REF_26_PDF_48MHZ          (7   << PDF_SHIFT)
#define REF_26_MFD_48MHZ          (506 << MFD_SHIFT)
#define REF_26_MFN_48MHZ          (195 << MFN_SHIFT)
#define REF_26_MFI_52MHZ          (5  << MFI_SHIFT)
#define REF_26_PDF_52MHZ          (4  << PDF_SHIFT)
#define REF_26_MFD_52MHZ          (2096774  << MFD_SHIFT)
#define REF_26_MFN_52MHZ          (0  << MFN_SHIFT)
#define REF_26_MFI_104MHZ         (8  << MFI_SHIFT)
#define REF_26_PDF_104MHZ         (3  << PDF_SHIFT)
#define REF_26_MFD_104MHZ         (2096774  << MFD_SHIFT)
#define REF_26_MFN_104MHZ         (0  << MFN_SHIFT)
#define REF_26_MFI_156MHZ         (12  << MFI_SHIFT)
#define REF_26_PDF_156MHZ         (3  << PDF_SHIFT)
#define REF_26_MFD_156MHZ         (2096774  << MFD_SHIFT)
#define REF_26_MFN_156MHZ         (0  << MFN_SHIFT)
#define REF_26_MFI_208MHZ         (12  << MFI_SHIFT)
#define REF_26_PDF_208MHZ         (2  << PDF_SHIFT)
#define REF_26_MFD_208MHZ         (8388606  << MFD_SHIFT)
#define REF_26_MFN_208MHZ         (0  << MFN_SHIFT)
#define REF_26_MFI_260MHZ         (15  << MFI_SHIFT)
#define REF_26_PDF_260MHZ         (2  << PDF_SHIFT)
#define REF_26_MFD_260MHZ         (8388606  << MFD_SHIFT)
#define REF_26_MFN_260MHZ         (0  << MFN_SHIFT)

#define DPLL_CLOCK_RATE_26_MHZ    HAPI_CLOCK_RATES_260_MHZ
#define RDD_DIVIDE_26_MHZ         DPLL_DIVIDE_BY_10
#define MCD_DIVIDE_26_MHZ         DPLL_DIVIDE_BY_10

#define DPLL_CLOCK_RATE_39_MHZ    HAPI_CLOCK_RATES_156_MHZ
#define RDD_DIVIDE_39_MHZ         DPLL_DIVIDE_BY_6
#define MCD_DIVIDE_39_MHZ         DPLL_DIVIDE_BY_4

#define DPLL_CLOCK_RATE_52_MHZ    HAPI_CLOCK_RATES_260_MHZ
#define RDD_DIVIDE_52_MHZ         DPLL_DIVIDE_BY_10
#define MCD_DIVIDE_52_MHZ         DPLL_DIVIDE_BY_5

typedef UINT8 HAPI_CLOCK_RATE_T;

typedef struct {
    UINT16 reserved1;
    UINT8  load;
    UINT8  reserved2;
} REF_DPLL_CTRL_BYTE_ACCESS_T;

typedef union {
    UINT32                      word_access;
    REF_DPLL_CTRL_BYTE_ACCESS_T byte_access;
} REF_DPLL_CTRL_T;

typedef struct {
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
    UINT32 ref_mfn_minus;
    UINT32 ref_mfn_plus;
    UINT32 mfn_tog_cnt;
    UINT32 desense_stat;
} HAPI_CLOCK_REG_T;

typedef struct {
	UINT8  mfi;
	UINT8  pdf;
	UINT32 mfd;
	UINT32 mfn;
} DPLL_REG_VALUE_TBL_T;

static const DPLL_REG_VALUE_TBL_T REF_26_DPLL_REG_VALUES[HAPI_DPLL_RATES_MAX] = {
	/* Invalid */   { MFI_INVALID,         PDF_INVALID,         MFD_INVALID,         MFN_INVALID         },
	/* 13    MHz */ { REF_26_MFI_13MHZ,    REF_26_PDF_13MHZ,    REF_26_MFD_13MHZ,    REF_26_MFN_13MHZ    },
	/* 16.8  MHz */ { REF_26_MFI_16_8MHZ,  REF_26_PDF_16_8MHZ,  REF_26_MFD_16_8MHZ,  REF_26_MFN_16_8MHZ  },
	/* 19.44 MHz */ { REF_26_MFI_19_44MHZ, REF_26_PDF_19_44MHZ, REF_26_MFD_19_44MHZ, REF_26_MFN_19_44MHZ },
	/* 26    MHz */ { REF_26_MFI_26MHZ,    REF_26_PDF_26MHZ,    REF_26_MFD_26MHZ,    REF_26_MFN_26MHZ    },
	/* 39    MHz */ { REF_26_MFI_39MHZ,    REF_26_PDF_39MHZ,    REF_26_MFD_39MHZ,    REF_26_MFN_39MHZ    },
	/* 40    MHz */ { REF_26_MFI_40MHZ,    REF_26_PDF_40MHZ,    REF_26_MFD_40MHZ,    REF_26_MFN_40MHZ    },
	/* 48    MHz */ { REF_26_MFI_48MHZ,    REF_26_PDF_48MHZ,    REF_26_MFD_48MHZ,    REF_26_MFN_48MHZ    },
	/* 52    MHz */ { REF_26_MFI_52MHZ,    REF_26_PDF_52MHZ,    REF_26_MFD_52MHZ,    REF_26_MFN_52MHZ    },
	/* 104   MHz */ { REF_26_MFI_104MHZ,   REF_26_PDF_104MHZ,   REF_26_MFD_104MHZ,   REF_26_MFN_104MHZ   },
	/* 156   MHz */ { REF_26_MFI_156MHZ,   REF_26_PDF_156MHZ,   REF_26_MFD_156MHZ,   REF_26_MFN_156MHZ   },
	/* 208   MHz */ { REF_26_MFI_208MHZ,   REF_26_PDF_208MHZ,   REF_26_MFD_208MHZ,   REF_26_MFN_208MHZ   },
	/* 260   MHz */ { REF_26_MFI_260MHZ,   REF_26_PDF_260MHZ,   REF_26_MFD_260MHZ,   REF_26_MFN_260MHZ   }
};

/* Clock Control Module (CCM) peripherals address. */
HAPI_CLOCK_REG_T *hapi_clock_reg = (void *) HAPI_CLOCK_REG_ADDRESS;

/* Current MCU clock rate address. */
HAPI_CLOCK_RATE_T *hapi_clock_rate_mcu = (void *) HAPI_CLOCK_RATE_ADDRESS;

extern UINT32 SetNeptuneClocks(NEPTUNE_CLOCKS_T neptune_clock) {
	UINT32 status;
	HAPI_CLOCK_RATE_T dpll_clock_rate;
	const DPLL_REG_VALUE_TBL_T *dpll_reg_value_tbl_ptr;

	status = RESULT_OK;
	dpll_clock_rate = 0;
	dpll_reg_value_tbl_ptr = NULL;

	switch (neptune_clock) {
		case CLOCKS_13MHZ_26MHZ: /* 13000000 hz  - 1410 - 83  */
			*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_13_MHZ;
			hapi_clock_reg->clk_sel &= ~MCU_CLK_SEL_MASK;
			hapi_clock_reg->clk_sel &= ~REF_SEL_MASK;
			hapi_clock_reg->clk_sel |= CODEC_CLK_SEL_MASK;
			break;
		case CLOCKS_26MHZ_26MHZ: /* 25999996 hz  - 1670 - 242 */
			*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_26_MHZ;
			dpll_clock_rate = DPLL_CLOCK_RATE_26_MHZ; /* 260 MHz */
			hapi_clock_reg->div_factor = /* 0x0600 (260 / 10) = 26 MHz */
				(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | (RDD_DIVIDE_26_MHZ << REF_DPLL_DIV_SHIFT);
			hapi_clock_reg->div_factor = /* 0x0006 (260 / 10) = 26 MHz */
				(hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | (MCD_DIVIDE_26_MHZ << MCU_DPLL_DIV_SHIFT);
			break;
		case CLOCKS_39MHZ_26MHZ: /* 38999965  hz - 1154 - 195 */
			*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_39_MHZ;
			dpll_clock_rate = DPLL_CLOCK_RATE_39_MHZ; /* 156 MHz */
			hapi_clock_reg->div_factor = /* 0x0400  (156 / 6) = 26 MHz */
				(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | (RDD_DIVIDE_39_MHZ << REF_DPLL_DIV_SHIFT);
			hapi_clock_reg->div_factor = /* 0x0002  (156 / 4) = 39 MHz */
				(hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | (MCD_DIVIDE_39_MHZ << MCU_DPLL_DIV_SHIFT);
			break;
		case CLOCKS_52MHZ_26MHZ: /* 51999983  hz - 1667 - 242 */
			*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_52_MHZ;
			dpll_clock_rate = DPLL_CLOCK_RATE_52_MHZ; /* 260 MHz */
			hapi_clock_reg->div_factor = /* 0x0600 (260 / 10) = 26 MHz */
				(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | (RDD_DIVIDE_52_MHZ << REF_DPLL_DIV_SHIFT);
			hapi_clock_reg->div_factor = /* 0x0003  (260 / 5) = 52 MHz */
				(hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | (MCD_DIVIDE_52_MHZ << MCU_DPLL_DIV_SHIFT);
			break;
		case CLOCKS_65MHZ_26MHZ: /* 64999987  hz - 1666 - 242 */
			*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_52_MHZ;
			dpll_clock_rate = DPLL_CLOCK_RATE_52_MHZ;
			hapi_clock_reg->div_factor = /* 0x0600 (260 / 10) = 26 MHz */
				(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | (RDD_DIVIDE_52_MHZ << REF_DPLL_DIV_SHIFT);
			hapi_clock_reg->div_factor = /* 0x0002  (260 / 4) = 65 MHz */
				(hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | (DPLL_DIVIDE_BY_4  << MCU_DPLL_DIV_SHIFT);
			break;
		case CLOCKS_65MHZ_32MHZ: /* 64999996  hz - 1410 - 242 */
			*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_52_MHZ;
			dpll_clock_rate = DPLL_CLOCK_RATE_52_MHZ;
			hapi_clock_reg->div_factor = /* 0x0500  (260 / 8) = 32.5 MHz */
				(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | (DPLL_DIVIDE_BY_8  << REF_DPLL_DIV_SHIFT);
			hapi_clock_reg->div_factor = /* 0x0002  (260 / 4) = 65 MHz   */
				(hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | (DPLL_DIVIDE_BY_4  << MCU_DPLL_DIV_SHIFT);
			break;
		case CLOCKS_65MHZ_43MHZ: /* 64999993  hz - 1154 - 242 */
			*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_52_MHZ;
			dpll_clock_rate = DPLL_CLOCK_RATE_52_MHZ;
			hapi_clock_reg->div_factor = /* 0x0400  (260 / 6) = 43.3(3) MHz */
				(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | (DPLL_DIVIDE_BY_6  << REF_DPLL_DIV_SHIFT);
			hapi_clock_reg->div_factor = /* 0x0002  (260 / 4) = 65 MHz      */
				(hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | (DPLL_DIVIDE_BY_4  << MCU_DPLL_DIV_SHIFT);
			break;
		case CLOCKS_69MHZ_26MHZ: /* UNKNOWN */
			*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_52_MHZ;
			dpll_clock_rate = HAPI_CLOCK_RATES_208_MHZ; /* 208 MHz */
			hapi_clock_reg->div_factor = /* 0x0500  (208 / 8) = 26 MHz      */
				(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | (DPLL_DIVIDE_BY_8  << REF_DPLL_DIV_SHIFT);
			hapi_clock_reg->div_factor = /* 0x0001  (208 / 3) = 69.3(3) MHz */
				(hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | (DPLL_DIVIDE_BY_3  << MCU_DPLL_DIV_SHIFT);
			break;
		case CLOCKS_78MHZ_26MHZ: /* UNKNOWN */
			*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_52_MHZ;
			dpll_clock_rate = HAPI_CLOCK_RATES_156_MHZ; /* 156 MHz */
			hapi_clock_reg->div_factor = /* 0x0400  (156 / 6) = 26 MH  */
				(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | (DPLL_DIVIDE_BY_6  << REF_DPLL_DIV_SHIFT);
			hapi_clock_reg->div_factor = /* 0x0000  (156 / 2) = 78 MHz */
				(hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | (DPLL_DIVIDE_BY_2  << MCU_DPLL_DIV_SHIFT);
			break;
		case CLOCKS_86MHZ_26MHZ: /* 86666634  hz - 1153 - 242 */
			*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_52_MHZ;
			dpll_clock_rate = DPLL_CLOCK_RATE_52_MHZ;
			hapi_clock_reg->div_factor = /* 0x0600 (260 / 10) = 26 MHz      */
				(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | (RDD_DIVIDE_52_MHZ << REF_DPLL_DIV_SHIFT);
			hapi_clock_reg->div_factor = /* 0x0001  (260 / 3) = 86.6(6) MHz */
				(hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | (DPLL_DIVIDE_BY_3  << MCU_DPLL_DIV_SHIFT);
			break;
		case CLOCKS_104MHZ_26MHZ: /* UNKNOWN */
			*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_52_MHZ;
			dpll_clock_rate = HAPI_CLOCK_RATES_208_MHZ; /* 208 MHz */
			hapi_clock_reg->div_factor = /* 0x0500  (208 / 8) = 26 MHz  */
				(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | (DPLL_DIVIDE_BY_8  << REF_DPLL_DIV_SHIFT);
			hapi_clock_reg->div_factor = /* 0x0000  (208 / 2) = 104 MHz */
				(hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | (DPLL_DIVIDE_BY_2  << MCU_DPLL_DIV_SHIFT);
			break;
		case CLOCKS_130MHZ_26MHZ: /* 129999963 hz - 640  - 242 */
			*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_52_MHZ;
			dpll_clock_rate = DPLL_CLOCK_RATE_52_MHZ;
			hapi_clock_reg->div_factor = /* 0x0600 (260 / 10) = 26 MHz  */
				(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | (RDD_DIVIDE_52_MHZ << REF_DPLL_DIV_SHIFT);
			hapi_clock_reg->div_factor = /* 0x0000  (260 / 2) = 130 MHz */
				(hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | (DPLL_DIVIDE_BY_2  << MCU_DPLL_DIV_SHIFT);
			break;
		default:
			break;
	}

	LOG("hapi_clock_rate_mcu: %d\n", *hapi_clock_rate_mcu);
	LOG("hapi_clock_reg->div_factor: %d\n", hapi_clock_reg->div_factor);

	dpll_reg_value_tbl_ptr = REF_26_DPLL_REG_VALUES;

	hapi_clock_reg->ref_dpll_op =
		dpll_reg_value_tbl_ptr[dpll_clock_rate].mfi | dpll_reg_value_tbl_ptr[dpll_clock_rate].pdf;
	hapi_clock_reg->ref_dpll_mfd = dpll_reg_value_tbl_ptr[dpll_clock_rate].mfd;
	hapi_clock_reg->ref_dpll_mfn = dpll_reg_value_tbl_ptr[dpll_clock_rate].mfn;

	hapi_clock_reg->clk_monitor &= (~PWR_DWN_AMP_MASK);
	hapi_clock_reg->ref_dpll_ctrl.word_access |= (DPLL_ENABLE_MASK | DPLL_BIN_RATE_MOD_MASK);

	/* while (((hapi_clock_reg->ref_dpll_ctrl.word_access & DPLL_LOCK_FLAG_MASK) == DPLL_LOCK_FLAG_MASK) == FALSE); */

	hapi_clock_reg->clk_sel = (hapi_clock_reg->clk_sel & ~DIV_CLK_SEL_MASK) | (CCM_REF_DPLL_SEL << DIV_CLK_SEL_SHIFT);
	hapi_clock_reg->clk_sel = (hapi_clock_reg->clk_sel & ~MCU_CLK_SEL_MASK) | (MCU_CLK_IN_SEL << MCU_CLK_SEL_SHIFT);
	hapi_clock_reg->clk_sel |= REF_SEL_MASK;
	hapi_clock_reg->clk_sel &= ~CODEC_CLK_SEL_MASK;

	return status;
}

extern const WCHAR *DetermineNeptuneMcuClock(void) {
	switch (*hapi_clock_rate_mcu) {
		case HAPI_CLOCK_RATES_13_MHZ:
				return L"13 MHz";
		case HAPI_CLOCK_RATES_26_MHZ:
				return L"26 MHz";
		case HAPI_CLOCK_RATES_39_MHZ:
				return L"39 MHz";
		case HAPI_CLOCK_RATES_52_MHZ:
				switch (hapi_clock_reg->div_factor) {
					case 1667:
						return L"52 MHz";
					case 1666:
					case 1410:
					case 1154:
						return L"65 MHz";
				}
			break;
	}
	return L"? MHz";
}
