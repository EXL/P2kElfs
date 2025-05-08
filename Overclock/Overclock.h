#ifndef OVERCLOCK_H
#define OVERCLOCK_H

#include <typedefs.h>

#ifdef ARGON

typedef enum {
	CORE_NORMAL = 385000000,
	CORE_TURBO = 514000000
} dvfs_op_point_t;

#define AHB_FREQ        128500000

#define MXC_CCM_BASE    0x53F80000
#define MXC_CCM_MCR     (MXC_CCM_BASE + 0x0)
#define MXC_CCM_MPDR0   (MXC_CCM_BASE + 0x4)
#define MXC_CCM_PMCR0   (MXC_CCM_BASE + 0x38)

#define MXC_CCM_MCR_TPE (0x1 << 4)
#define MXC_CCM_MCR_TPL (0x1 << 3)
#define MXC_CCM_MCR_MPL (0x1 << 2)
#define MXC_CCM_PMCR0_DFSP 0x02000000
#define MXC_CCM_PMCR0_DFSI 0x01000000

#define MEGA_HERTZ      1000000
#define ERR_DFSP_SWITCH 2

#define MAX_PDF_3       0x00000010
#define MAX_PDF_4       0x00000018
#define MAX_PDF_5       0x00000020
#define MPDR0_BRMM_0    0x00000000

#define MXC_CCM_MPDR0_TPSEL         (0x1 << 11)
#define MXC_CCM_MPDR0_MAX_PDF_MASK  (0x7 << 3)
#define MXC_CCM_MPDR0_BRMM_MASK     0x7
#define TURBO_MASK                  0x0000083F

typedef enum { TPSEL_CLEAR = 0, TPSEL_SET = 1 } tpsel_t;
typedef enum { DFS_SWITCH = 0, DFS_NO_SWITCH = 1 } dfs_switch_t;

#define __raw_readl(a)  (*(volatile unsigned int *)(a))
#define __raw_writel(v, a) (*(volatile unsigned int *)(a) = (v))

extern const WCHAR *DetermineArgonLVClock(void);
extern UINT32 SetArgonLVClocks(dvfs_op_point_t dvfs_op);

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
