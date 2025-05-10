#ifndef OVERCLOCK_H
#define OVERCLOCK_H

#include <typedefs.h>

#ifdef ARGON

typedef enum {
	CORE_NORMAL = 385500000,
	CORE_TURBO = 514000000,
	CORE_TURBO_1 = 532000000,
	CORE_TURBO_2 = 548000000,
	CORE_TURBO_3 = 642460000,
	CORE_TURBO_4 = 680000000,
} dvfs_op_point_t;

enum plls {
	MCUPLL = 0,
	USBPLL = 2,
	DSPPLL = 1,
	CDPLL = 3,
	CKIH = 4,
	CKIH_X2 = 5,
	USBCLK = 8,
	TURBOPLL,
	SERIALPLL,
};

#define CORE_DVFS_PLL                       2
#define DVFS_OP_NUM                         5

#define AHB_FREQ        128500000
#define AHB_FREQ_MAX			    128500000
#define MXC_CKIH_FREQ                       26000000
#define MXC_PLL_REF_CLK                     MXC_CKIH_FREQ

#define IIM_BASE_ADDR           0x5001C000
#define SYSTEM_PREV_REG       IIM_BASE_ADDR + 0x20
#define SYSTEM_SREV_REG       IIM_BASE_ADDR + 0x24
#define IIM_PROD_REV_SH       3
#define IIM_PROD_REV_LEN      5

#define CHIP_REV_1_0            0x10
#define CHIP_REV_1_1		0x11
#define CHIP_REV_1_2		0x12
#define CHIP_REV_1_2_2		0x13
#define CHIP_REV_2_0            0x20
#define CHIP_REV_2_1            0x21
#define CHIP_REV_2_3            0x22
#define CHIP_REV_2_3_2          0x23

#define PROD_SIGNATURE          0x6
#define SYSTEM_REV_MIN          CHIP_REV_1_0
#define SYSTEM_REV_NUM          6

#define MXC_CCM_BASE    0x53F80000
#define MXC_CCM_MCR     (MXC_CCM_BASE + 0x0)
#define MXC_CCM_MPDR0   (MXC_CCM_BASE + 0x4)
#define MXC_CCM_MPDR1		(MXC_CCM_BASE + 0x8)
#define MXC_CCM_MPCTL		(MXC_CCM_BASE + 0x10)
#define MXC_CCM_UPCTL		(MXC_CCM_BASE + 0x14)
#define MXC_CCM_MCGR0		(MXC_CCM_BASE + 0x1C)
#define MXC_CCM_MCGR1		(MXC_CCM_BASE + 0x20)
#define MXC_CCM_PMCR0   (MXC_CCM_BASE + 0x38)
#define MXC_CCM_TPCTL		(MXC_CCM_BASE + 0x44)
#define MXC_CCM_MPDR2		(MXC_CCM_BASE + 0x50)

#define MXC_CCM_MCR_TPE (0x1 << 4)
#define MXC_CCM_MCR_TPL (0x1 << 3)
#define MXC_CCM_MCR_MPL (0x1 << 2)
#define MXC_CCM_MCR_FIRS                    (0x1 << 11)
#define MXC_CCM_PMCR0_DFSP 0x02000000
#define MXC_CCM_PMCR0_DFSI 0x01000000
#define MXC_CCM_MPDR0_MAX_PDF_OFFSET 3
#define MXC_CCM_MCR_SSIS1                   (0x1 << 28)
#define MXC_CCM_MCR_SSIS2                   (0x1 << 27)
#define MXC_CCM_MPDR0_IPG_PDF_MASK          (0x3 << 6)
#define MXC_CCM_MPDR1_FIRI_PREPDF_OFFSET    24
#define MXC_CCM_MPDR1_FIRI_PREPDF_MASK      (0x7 << 24)
#define MXC_CCM_MPDR1_FIRI_PDF_OFFSET       19
#define MXC_CCM_MPDR1_FIRI_PDF_MASK         (0x1F << 19)
#define MXC_CCM_MPDR0_CSI_FPDF              (0x1 << 22)
#define MXC_CCM_MPDR1_FIRI_DIS              (0x1 << 18)
#define MXC_CCM_MPDR0_CSI_PRE               (0x1 << 21)
#define MXC_CCM_MPDR0_CSI_DIS               (0x1 << 19)
#define MXC_CCM_MPDR0_CSI_PDF_OFFSET        23
#define MXC_CCM_MPDR0_CSI_PDF_MASK          (0x1FF << 23)
#define MXC_CCM_MPDR1_SSI2_PDF_OFFSET       10
#define MXC_CCM_MPDR1_SSI2_PDF_MASK         (0x1F << 10)
#define MXC_CCM_MPDR1_SSI2_PREPDF_OFFSET    15
#define MXC_CCM_MPDR1_SSI2_PREPDF_MASK      (0x7 << 15)
#define MXC_CCM_MPDR1_SSI1_PDF_OFFSET       1
#define MXC_CCM_MPDR1_SSI1_PDF_MASK         (0x1F << 1)
#define MXC_CCM_MPDR1_SSI2_DIS              (0x1 << 9)
#define MXC_CCM_MPDR1_SSI1_PREPDF_OFFSET    6
#define MXC_CCM_MPDR1_SSI1_PREPDF_MASK      (0x7 << 6)
#define MXC_CCM_MPDR1_SSI1_DIS              0x1
#define MXC_CCM_MPDR1_USB_PDF_OFFSET        27
#define MXC_CCM_MPDR1_USB_PDF_MASK          (0x7 << 27)
#define MXC_CCM_MPDR0_NFC_PDF_OFFSET        8
#define MXC_CCM_MPDR0_NFC_PDF_MASK          (0x7 << 8)
#define MXC_CCM_MPDR1_USB_DIS               (0x1 << 30)
#define MXC_CCM_MPDR2_SDHC2_PDF_OFFSET      7
#define MXC_CCM_MPDR2_SDHC2_PDF_MASK        (0xF << 7)
#define MXC_CCM_MPDR2_SDHC2DIS              (0x1 << 6)
#define MXC_CCM_MPDR2_SDHC1_PDF_OFFSET      1
#define MXC_CCM_MPDR2_SDHC1_PDF_MASK        (0xF << 1)
#define MXC_CCM_MPDR2_SDHC1DIS              0x1
#define MXC_CCM_MPDR0_TPSEL_OFFSET          11
#define MXC_CCM_MPDR0_IPG_PDF_OFFSET        6
#define MXC_CCM_MCR_SDHC2S_OFFSET           9
#define MXC_CCM_MCR_SDHC2S_MASK             (0x3 << 9)
#define MXC_CCM_MCR_SDHC1S_OFFSET           14
#define MXC_CCM_MCR_SDHC1S_MASK             (0x3 << 14)
#define MXC_CCM_MCR_CSIS_OFFSET             25
#define MXC_CCM_MCR_CSIS_MASK               (0x3 << 25)
#define MXC_CCM_PCTL_MFD_OFFSET             16
#define MXC_CCM_PCTL_MFD_MASK               (0x3FF << 16)
#define MXC_CCM_PCTL_PDF_OFFSET             26
#define MXC_CCM_PCTL_PDF_MASK               (0xF << 26)
#define MXC_CCM_TPCTL_MFI_OFFSET            11
#define MXC_CCM_TPCTL_MFI_MASK              (0xF << 11)
#define MXC_CCM_TPCTL_MFN_OFFSET            0
#define MXC_CCM_TPCTL_MFN_MASK              0x7FF
#define MXC_CCM_USBPCTL_MFI_OFFSET          10
#define MXC_CCM_USBPCTL_MFI_MASK            (0xF << 10)
#define MXC_CCM_USBPCTL_MFN_OFFSET          0
#define MXC_CCM_USBPCTL_MFN_MASK            0x3FF
#define MXC_CCM_MCUPCTL_MFI_OFFSET          11
#define MXC_CCM_MCUPCTL_MFI_MASK            (0xF << 11)
#define MXC_CCM_MCUPCTL_MFN_OFFSET          0
#define MXC_CCM_MCUPCTL_MFN_MASK            0x7FF

#define MXC_CCM_MCGR1_SDHC1_EN              (0x2 << 28)
#define MXC_CCM_MCGR1_SDHC1_MASK            (0x3 << 28)
#define MXC_CCM_MCGR0_UART1_EN              (0x2 << 4)
#define MXC_CCM_MCGR0_UART1_MASK            (0x3 << 4)

#define MXC_CCM_PMCR0_PTVAIM                0x00000008

#define MXC_TIMER_CLK           32768
#define MXC_TIMER_DIVIDER	1
#define MEGA_HERTZ      1000000
#define ERR_DFSP_SWITCH 2

#define MAX_PDF_3       0x00000010
#define MAX_PDF_4       0x00000018
#define MAX_PDF_5       0x00000020
#define MPDR0_BRMM_0            0x00000000
#define MPDR0_BRMM_1            0x00000001
#define MPDR0_BRMM_2            0x00000002
#define MPDR0_BRMM_3            0x00000003
#define MPDR0_BRMM_4            0x00000004


#define MXC_CCM_MPDR0_TPSEL         (0x1 << 11)
#define MXC_CCM_MPDR0_MAX_PDF_MASK  (0x7 << 3)
#define MXC_CCM_MPDR0_BRMM_MASK     0x7
#define TURBO_MASK                  0x0000083F

typedef enum { TPSEL_CLEAR = 0, TPSEL_SET = 1 } tpsel_t;
typedef enum { DFS_SWITCH = 0, DFS_NO_SWITCH = 1 } dfs_switch_t;

#define __raw_readl(a)  (*(volatile unsigned int *)(a))
#define __raw_writel(v, a) (*(volatile unsigned int *)(a) = (v))

enum mxc_clocks {
	CLK_NONE,
	CKIL_CLK,
	CKIH_CLK,
	CPU_CLK,
	AHB_CLK,
	IPG_CLK,
	NFC_CLK,
	USB_CLK,
	USB_IPG_CLK,
	USB_AHB_CLK,
	UART1_BAUD,
	UART2_BAUD,
	UART3_BAUD,
	UART4_BAUD,
	UART5_BAUD,
	UART6_BAUD,
	SSI1_BAUD,
	SSI2_BAUD,
	SSI1_IPG_CLK,
	SSI2_IPG_CLK,
	CSI_BAUD,
	FIRI_BAUD,
	I2C_CLK,
	I2C1_CLK = I2C_CLK,
	I2C2_CLK,
	I2C3_CLK,
	CSPI1_CLK,
	CSPI2_CLK,
	CSPI3_CLK,
	GPT_CLK,
	RTC_CLK,
	EPIT1_CLK,
	EPIT2_CLK,
	EDIO_CLK,
	WDOG_CLK,
	WDOG2_CLK,
	PWM_CLK,
	IPU_CLK,
	SIM1_CLK,
	SIM2_CLK,
	HAC_CLK,
	GEM_CLK,
	SDHC1_CLK,
	SDHC2_CLK,
	SDMA_CLK,
	RNG_CLK,
	KPP_CLK,
	MU_CLK,
	RTIC_CLK,
	SCC_CLK,
	SPBA_CLK,
	DSM_CLK,
	SAHARA2_IPG_CLK,
	SAHARA2_AHB_CLK,
	MQSPI_IPG_CLK,
	MQSPI_CKIH_CLK,
	EL1T_IPG_CLK,
	EL1T_NET_CLK,
	LPMC_CLK,
	MPEG4_CLK,
	OWIRE_CLK,
	MBX_CLK,
	MSTICK1_BAUD,
	MSTICK2_BAUD,
	ATA_CLK,
	IIM_CLK,
	SMC_CLK,
	ECT_CLK,
	RTRMCU_CLK,
	EMI_CLK,
	SAHARA_CLK,
};

extern UINT32 SetArgonLVClocks(dvfs_op_point_t dvfs_op);
int SetArgonLVTurboMode(unsigned long max_pdf);
void PrintArgonLVCpuInfo(void);

extern const WCHAR* DetermineArgonLVClock(void);

tpsel_t mxc_pm_chk_tpsel(void);
dfs_switch_t mxc_pm_chk_dfsp(void);
void mxc_ccm_modify_reg(unsigned int reg_offset, unsigned int mask, unsigned int data);
void mxc_pm_setbrmm(int value);
unsigned long mxc_pll_clock(enum plls pll);
unsigned long mxc_get_clocks(enum mxc_clocks clk);
unsigned long mxc_get_clocks_parent(enum mxc_clocks clk);
int mxc_pm_setturbo(dvfs_op_point_t dvfs_op_reqd, unsigned long max_pdf);
void mxc_set_clocks_pll(enum mxc_clocks clk, enum plls pll_num);
void mxc_clk_getdivs(unsigned int div, unsigned int *div1, unsigned int *div2);
void mxc_clks_enable(enum mxc_clocks clk);
void mxc_set_clocks_div(enum mxc_clocks clk, unsigned int div);
unsigned long mxc_mcu_active_pll_clk(void);

#else
typedef enum {
	CLOCKS_13MHZ_13MHZ,
	CLOCKS_13MHZ_26MHZ,
	CLOCKS_26MHZ_21MHZ,
	CLOCKS_26MHZ_26MHZ,
	CLOCKS_39MHZ_26MHZ,
	CLOCKS_52MHZ_26MHZ,
	CLOCKS_65MHZ_26MHZ,
	CLOCKS_65MHZ_32MHZ,
	CLOCKS_65MHZ_43MHZ,
	CLOCKS_69MHZ_26MHZ,
	CLOCKS_78MHZ_26MHZ,
	CLOCKS_86MHZ_26MHZ,
	CLOCKS_104MHZ_26MHZ,
	CLOCKS_130MHZ_26MHZ
} NEPTUNE_CLOCKS_T;

extern UINT32 SetNeptuneClocks(NEPTUNE_CLOCKS_T neptune_clock);

extern const WCHAR *DetermineNeptuneMcuClock(void);
#endif
#endif // OVERCLOCK_H
