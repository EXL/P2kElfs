#include <loader.h>
#include <utilities.h>
#include <dl.h>

#include "Overclock.h"

#ifdef ARGON

tpsel_t mxc_pm_chk_tpsel(void) {
    return (__raw_readl(MXC_CCM_MPDR0) & MXC_CCM_MPDR0_TPSEL) ? TPSEL_SET : TPSEL_CLEAR;
}

dfs_switch_t mxc_pm_chk_dfsp(void) {
    return (__raw_readl(MXC_CCM_PMCR0) & MXC_CCM_PMCR0_DFSP) ? DFS_SWITCH : DFS_NO_SWITCH;
}

void mxc_ccm_modify_reg(unsigned int reg_offset, unsigned int mask, unsigned int data) {
    unsigned int reg = __raw_readl(reg_offset);
    reg = (reg & ~mask) | data;
    __raw_writel(reg, reg_offset);
}

void mxc_pm_setbrmm(int value) {
    mxc_ccm_modify_reg(MXC_CCM_MPDR0, MXC_CCM_MPDR0_BRMM_MASK, value);
}

unsigned int core_pll_op_100[CORE_DVFS_PLL][DVFS_OP_NUM] = {
    {399000000, 200000000, 160000000, 133000000, 100000000},
    {500000000, 214000000, 166000000, 136000000, 100000000}
};

unsigned int core_pll_op_128[CORE_DVFS_PLL][DVFS_OP_NUM] = {
    {385000000, 231000000, 193000000, 165000000, 128000000},
    {514000000, 257000000, 206000000, 171000000, 128000000}
};

unsigned long system_rev_tbl[SYSTEM_REV_NUM][2] = {
	{0x00, CHIP_REV_1_0},
	{0x11, CHIP_REV_1_1},
	{0x22, CHIP_REV_1_2},
	{0x24, CHIP_REV_1_2_2},
	{0x50, CHIP_REV_2_3},
	{0x52, CHIP_REV_2_3_2},
};

unsigned long mxc_pll_clock(enum plls pll) {
    unsigned long mfi = 0, mfn = 0, mfd, pdf, ref_clk;
    unsigned long long temp;
    unsigned long reg;

    if (pll == MCUPLL) {
        reg = __raw_readl(MXC_CCM_MPCTL);
        mfi = (reg & MXC_CCM_MCUPCTL_MFI_MASK) >> MXC_CCM_MCUPCTL_MFI_OFFSET;
        mfi = mfi <= 5 ? 5 : mfi;
        mfn = reg & MXC_CCM_MCUPCTL_MFN_MASK;
        mfn = mfn <= 0x400 ? mfn : mfn - 0x800;
    } else if (pll == USBPLL) {
        reg = __raw_readl(MXC_CCM_UPCTL);
        mfi = (reg & MXC_CCM_USBPCTL_MFI_MASK) >> MXC_CCM_USBPCTL_MFI_OFFSET;
        mfi = mfi <= 5 ? 5 : mfi;
        mfn = reg & MXC_CCM_USBPCTL_MFN_MASK;
    } else if (pll == TURBOPLL) {
        reg = __raw_readl(MXC_CCM_TPCTL);
        mfi = (reg & MXC_CCM_TPCTL_MFI_MASK) >> MXC_CCM_TPCTL_MFI_OFFSET;
        mfi = mfi <= 5 ? 5 : mfi;
        mfn = reg & MXC_CCM_TPCTL_MFN_MASK;
        mfn = mfn <= 0x400 ? mfn : mfn - 0x800;
    }

    pdf = (reg & MXC_CCM_PCTL_PDF_MASK) >> MXC_CCM_PCTL_PDF_OFFSET;
    mfd = (reg & MXC_CCM_PCTL_MFD_MASK) >> MXC_CCM_PCTL_MFD_OFFSET;
    ref_clk = MXC_PLL_REF_CLK;

    if (mfn < 1024) {
        temp = (unsigned long long)2 * ref_clk * mfn;
        temp /= mfd + 1;
        temp += (unsigned long long)2 * ref_clk * mfi;
        temp /= pdf + 1;
    } else {
        temp = (unsigned long long)2 * ref_clk * (2048 - mfn);
        temp /= mfd + 1;
        temp = (unsigned long long)2 * ref_clk * mfi - temp;
        temp /= pdf + 1;
    }

    return (unsigned long)temp;
}

unsigned long mxc_get_clocks_parent(enum mxc_clocks clk) {
    unsigned long ret_val = 0, clksel;
    unsigned long reg = __raw_readl(MXC_CCM_MCR);

    switch (clk) {
    case CSI_BAUD:
        clksel = (reg & MXC_CCM_MCR_CSIS_MASK) >> MXC_CCM_MCR_CSIS_OFFSET;
        ret_val = clksel == 0 ? mxc_pll_clock(USBPLL) :
                  clksel == 1 ? mxc_pll_clock(MCUPLL) :
                  clksel == 2 ? mxc_pll_clock(TURBOPLL) : MXC_CKIH_FREQ;
        break;
    case SDHC1_CLK:
        clksel = (reg & MXC_CCM_MCR_SDHC1S_MASK) >> MXC_CCM_MCR_SDHC1S_OFFSET;
        ret_val = clksel == 0 ? mxc_pll_clock(USBPLL) :
                  clksel == 1 ? mxc_pll_clock(MCUPLL) :
                  clksel == 2 ? mxc_pll_clock(TURBOPLL) : MXC_CKIH_FREQ;
        break;
    case SDHC2_CLK:
        clksel = (reg & MXC_CCM_MCR_SDHC2S_MASK) >> MXC_CCM_MCR_SDHC2S_OFFSET;
        ret_val = clksel == 0 ? mxc_pll_clock(USBPLL) :
                  clksel == 1 ? mxc_pll_clock(MCUPLL) :
                  clksel == 2 ? mxc_pll_clock(TURBOPLL) : MXC_CKIH_FREQ;
        break;
    case SSI1_BAUD:
        clksel = reg & MXC_CCM_MCR_SSIS1;
        ret_val = mxc_pll_clock(clksel);
        break;
    case SSI2_BAUD:
        clksel = reg & MXC_CCM_MCR_SSIS2;
        ret_val = mxc_pll_clock(clksel);
        break;
    case FIRI_BAUD:
        clksel = reg & MXC_CCM_MCR_FIRS;
        ret_val = mxc_pll_clock(clksel);
        break;
	default:
		break;
    }
    return ret_val;
}

int mxc_pm_setturbo(dvfs_op_point_t dvfs_op_reqd, unsigned long max_pdf) {
    unsigned int mpdr = __raw_readl(MXC_CCM_MPDR0);

    switch (dvfs_op_reqd) {
    case CORE_NORMAL:
        mpdr |= MXC_CCM_MPDR0_TPSEL;
        mpdr = (mpdr & ~MXC_CCM_MPDR0_MAX_PDF_MASK) | max_pdf;
        break;
    case CORE_TURBO:
        mpdr &= ~MXC_CCM_MPDR0_TPSEL;
        mpdr = (mpdr & ~MXC_CCM_MPDR0_MAX_PDF_MASK) | max_pdf;
        break;
    default:
        return 0;
    }

    mpdr = (mpdr & ~MXC_CCM_MPDR0_BRMM_MASK) | MPDR0_BRMM_0;
    if (mxc_pm_chk_dfsp() == DFS_NO_SWITCH) {
        mxc_ccm_modify_reg(MXC_CCM_MPDR0, TURBO_MASK, mpdr);
        return 0;
    }
    return ERR_DFSP_SWITCH;
}

extern const WCHAR* DetermineArgonLVClock(void) {

	unsigned long clock = mxc_pm_chk_tpsel() == TPSEL_CLEAR ? mxc_pll_clock(MCUPLL) : mxc_pll_clock(TURBOPLL);

	switch (clock) {
		case CORE_NORMAL:
			return L"385 MHz";
			break;
		case CORE_TURBO:
			return L"514 MHz";
			break;
		case CORE_TURBO_1:
			return L"532 MHz";
			break;
		case CORE_TURBO_2:
			return L"548 MHz";
			break;
		case CORE_TURBO_3:
			return L"642.5 MHz";
			break;
		case CORE_TURBO_4:
			return L"680 MHz";
			break;
		default:
			return L"Unknown";
			break;
	}
}

void mxc_set_clocks_pll(enum mxc_clocks clk, enum plls pll_num) {
    unsigned long mcr = __raw_readl(MXC_CCM_MCR);

    switch (clk) {
    case SSI1_BAUD:
        if (pll_num == USBPLL) {
			mcr |= MXC_CCM_MCR_SSIS1;
		} else {
			mcr &= ~MXC_CCM_MCR_SSIS1;
		}
        break;
    case SSI2_BAUD:
        if (pll_num == USBPLL) {
			mcr |= MXC_CCM_MCR_SSIS2;
		} else {
			mcr &= ~MXC_CCM_MCR_SSIS2;
		}
        break;
    case FIRI_BAUD:
        if (pll_num == USBPLL) {
			mcr |= MXC_CCM_MCR_FIRS;
		} else {
			mcr &= ~MXC_CCM_MCR_FIRS;
		}
        break;
    case CSI_BAUD:
        if (pll_num == USBPLL) {
            mcr &= ~MXC_CCM_MCR_CSIS_MASK;
        } else if (pll_num == MCUPLL) {
            mcr = (mcr & ~MXC_CCM_MCR_CSIS_MASK) | (0x1 << MXC_CCM_MCR_CSIS_OFFSET);
        } else if (pll_num == TURBOPLL) {
            mcr = (mcr & ~MXC_CCM_MCR_CSIS_MASK) | (0x2 << MXC_CCM_MCR_CSIS_OFFSET);
        } else {
            mcr |= MXC_CCM_MCR_CSIS_MASK;
        }
        break;
    case SDHC1_CLK:
        if (pll_num == USBPLL) {
            mcr &= ~MXC_CCM_MCR_SDHC1S_MASK;
        } else if (pll_num == MCUPLL) {
            mcr = (mcr & ~MXC_CCM_MCR_SDHC1S_MASK) | (0x1 << MXC_CCM_MCR_SDHC1S_OFFSET);
        } else if (pll_num == TURBOPLL) {
            mcr = (mcr & ~MXC_CCM_MCR_SDHC1S_MASK) | (0x2 << MXC_CCM_MCR_SDHC1S_OFFSET);
        } else {
            mcr |= MXC_CCM_MCR_SDHC1S_MASK;
        }
        break;
    case SDHC2_CLK:
        if (pll_num == USBPLL) {
            mcr &= ~MXC_CCM_MCR_SDHC2S_MASK;
        } else if (pll_num == MCUPLL) {
            mcr = (mcr & ~MXC_CCM_MCR_SDHC2S_MASK) | (0x1 << MXC_CCM_MCR_SDHC2S_OFFSET);
        } else if (pll_num == TURBOPLL) {
            mcr = (mcr & ~MXC_CCM_MCR_SDHC2S_MASK) | (0x2 << MXC_CCM_MCR_SDHC2S_OFFSET);
        } else {
            mcr |= MXC_CCM_MCR_SDHC2S_MASK;
        }
        break;
	default:
		LOG("This clock does not have ability to choose the PLL\n");
		break;
    }

    __raw_writel(mcr, MXC_CCM_MCR);
}

void mxc_clk_getdivs(unsigned int div, unsigned int *div1, unsigned int *div2) {
    int i;

    if (div <= 8) {
        *div1 = div;
        *div2 = 1;
        return;
    }
    if (div <= 32) {
        *div1 = 1;
        *div2 = div;
        return;
    }
    for (i = 2; i < 9; i++) {
        if (div % i == 0) {
            *div1 = i;
            *div2 = div / i;
            return;
        }
    }
    *div1 = 2;
    *div2 = 12;
}

void mxc_clks_enable(enum mxc_clocks clk) {
    unsigned long reg;

    switch (clk) {
    case UART1_BAUD:
        reg = __raw_readl(MXC_CCM_MCGR0);
        reg = (reg & ~MXC_CCM_MCGR0_UART1_MASK) | MXC_CCM_MCGR0_UART1_EN;
        __raw_writel(reg, MXC_CCM_MCGR0);
        break;
    case SDHC1_CLK:
        reg = __raw_readl(MXC_CCM_MPDR2);
        reg &= ~MXC_CCM_MPDR2_SDHC1DIS;
        __raw_writel(reg, MXC_CCM_MPDR2);
        reg = __raw_readl(MXC_CCM_MCGR1);
        reg = (reg & ~MXC_CCM_MCGR1_SDHC1_MASK) | MXC_CCM_MCGR1_SDHC1_EN;
        __raw_writel(reg, MXC_CCM_MCGR1);
        break;
	default:
		LOG("The gateon for this clock(%d) is not implemented\n", clk);
		break;
    }
}

void mxc_set_clocks_div(enum mxc_clocks clk, unsigned int div) {
    unsigned long reg;
    unsigned int d, div1, div2;
    unsigned long pll;
    unsigned long csi_ref_freq = 288000000;
    int set_fpdf = 0;

    switch (clk) {
    case AHB_CLK:
        reg = __raw_readl(MXC_CCM_MPDR0);
        reg = (reg & ~MXC_CCM_MPDR0_MAX_PDF_MASK) | ((div - 1) << MXC_CCM_MPDR0_MAX_PDF_OFFSET);
        __raw_writel(reg, MXC_CCM_MPDR0);
        break;
    case IPG_CLK:
        reg = __raw_readl(MXC_CCM_MPDR0);
        reg = (reg & ~MXC_CCM_MPDR0_IPG_PDF_MASK) | ((div - 1) << MXC_CCM_MPDR0_IPG_PDF_OFFSET);
        __raw_writel(reg, MXC_CCM_MPDR0);
        break;
    case NFC_CLK:
        reg = __raw_readl(MXC_CCM_MPDR0);
        reg = (reg & ~MXC_CCM_MPDR0_NFC_PDF_MASK) | ((div - 1) << MXC_CCM_MPDR0_NFC_PDF_OFFSET);
        __raw_writel(reg, MXC_CCM_MPDR0);
        break;
    case CSI_BAUD:
        reg = __raw_readl(MXC_CCM_MPDR0);
        d = div / 2;
        if (div % 2 != 0) {
            set_fpdf = 1;
        }
        pll = mxc_get_clocks_parent(clk);
        if (pll > csi_ref_freq) {
            reg |= MXC_CCM_MPDR0_CSI_PRE;
            if (d % 2 != 0) {
                set_fpdf = 1;
            }
            d /= 2;
        } else {
            reg &= ~MXC_CCM_MPDR0_CSI_PRE;
        }
        if (set_fpdf == 1) {
            reg |= MXC_CCM_MPDR0_CSI_FPDF;
        } else {
            reg &= ~MXC_CCM_MPDR0_CSI_FPDF;
        }
        reg = (reg & ~MXC_CCM_MPDR0_CSI_PDF_MASK) | ((d - 1) << MXC_CCM_MPDR0_CSI_PDF_OFFSET);
        __raw_writel(reg, MXC_CCM_MPDR0);
        break;
    case USB_CLK:
        reg = __raw_readl(MXC_CCM_MPDR1);
        reg = (reg & ~MXC_CCM_MPDR1_USB_PDF_MASK) | ((div - 1) << MXC_CCM_MPDR1_USB_PDF_OFFSET);
        __raw_writel(reg, MXC_CCM_MPDR1);
        break;
    case SSI1_BAUD:
        reg = __raw_readl(MXC_CCM_MPDR1);
        d = div / 2;
        mxc_clk_getdivs(d, &div1, &div2);
        reg = (reg & ~MXC_CCM_MPDR1_SSI1_PREPDF_MASK) | ((div1 - 1) << MXC_CCM_MPDR1_SSI1_PREPDF_OFFSET);
        reg = (reg & ~MXC_CCM_MPDR1_SSI1_PDF_MASK) | ((div2 - 1) << MXC_CCM_MPDR1_SSI1_PDF_OFFSET);
        __raw_writel(reg, MXC_CCM_MPDR1);
        break;
    case SSI2_BAUD:
        reg = __raw_readl(MXC_CCM_MPDR1);
        d = div / 2;
        mxc_clk_getdivs(d, &div1, &div2);
        reg = (reg & ~MXC_CCM_MPDR1_SSI2_PREPDF_MASK) | ((div1 - 1) << MXC_CCM_MPDR1_SSI2_PREPDF_OFFSET);
        reg = (reg & ~MXC_CCM_MPDR1_SSI2_PDF_MASK) | ((div2 - 1) << MXC_CCM_MPDR1_SSI2_PDF_OFFSET);
        __raw_writel(reg, MXC_CCM_MPDR1);
        break;
    case FIRI_BAUD:
        reg = __raw_readl(MXC_CCM_MPDR1);
        d = div / 2;
        mxc_clk_getdivs(d, &div1, &div2);
        reg = (reg & ~MXC_CCM_MPDR1_FIRI_PREPDF_MASK) | ((div1 - 1) << MXC_CCM_MPDR1_FIRI_PREPDF_OFFSET);
        reg = (reg & ~MXC_CCM_MPDR1_FIRI_PDF_MASK) | ((div2 - 1) << MXC_CCM_MPDR1_FIRI_PDF_OFFSET);
        __raw_writel(reg, MXC_CCM_MPDR1);
        break;
    case SDHC1_CLK:
        reg = __raw_readl(MXC_CCM_MPDR2);
        reg = (reg & ~MXC_CCM_MPDR2_SDHC1_PDF_MASK) | ((div - 1) << MXC_CCM_MPDR2_SDHC1_PDF_OFFSET);
        __raw_writel(reg, MXC_CCM_MPDR2);
        break;
    case SDHC2_CLK:
        reg = __raw_readl(MXC_CCM_MPDR2);
        reg = (reg & ~MXC_CCM_MPDR2_SDHC2_PDF_MASK) | ((div - 1) << MXC_CCM_MPDR2_SDHC2_PDF_OFFSET);
        __raw_writel(reg, MXC_CCM_MPDR2);
        break;
	default:
		break;
    }
}

int SetArgonLVTurboMode(unsigned long max_pdf) {
	int retval = mxc_pm_setturbo(CORE_NORMAL, max_pdf);
	if (retval == ERR_DFSP_SWITCH) {
		return -ERR_DFSP_SWITCH;
	}
	while (!(__raw_readl(MXC_CCM_MCR) & MXC_CCM_MCR_TPL));
	int ms = 0;
	while (ms < 5000) {
		if (__raw_readl(MXC_CCM_PMCR0) & MXC_CCM_PMCR0_DFSI) {
			break;
		}
			suSleep(1, NULL);
			ms++;
		}
		if (ms >= 5000 && mxc_pm_chk_dfsp() == DFS_SWITCH) {
		return -1;
	}
	__raw_writel(__raw_readl(MXC_CCM_PMCR0) | MXC_CCM_PMCR0_DFSI, MXC_CCM_PMCR0);

	return 0;
}

void PrintArgonLVCpuInfo(void) {

	unsigned long val, i, rev = 0xFF;

	val = __raw_readl(SYSTEM_PREV_REG);
	LOG("SYSTEM_PREV_REG=0x%x\n", val);

	val = __raw_readl(SYSTEM_SREV_REG);
	LOG("SYSTEM_SREV_REG=0x%x\n", val);

	for (i = 0; i < SYSTEM_REV_NUM; i++) {
		if ((val & 0xFF) == system_rev_tbl[i][0]) {
			rev = system_rev_tbl[i][1];
			break;
		}
	}

	if(rev == 0xFF) {
		for (i = 0; i < SYSTEM_REV_NUM; i++) {
			if ((val & 0xF3) == system_rev_tbl[i][0]) {
				rev = system_rev_tbl[i][1];
				break;
			}
		}
	}

	if(rev == 0xFF) {
		for (i = 0; i < SYSTEM_REV_NUM; i++) {
			if ((val & 0xF0) == system_rev_tbl[i][0]) {
				rev = system_rev_tbl[i][1];
				break;
			}
		}
	}

	if (i == SYSTEM_REV_NUM) {
	    rev = system_rev_tbl[SYSTEM_REV_NUM - 1][1];
 	    LOG("WARNING: Can't find valid system rev\n");
	    LOG("Assuming last known system_rev=0x%x\n", rev);
	} else {
	    LOG("Processor system_rev=0x%x\n", rev);
	}
}

UINT32 SetArgonLVClocks(dvfs_op_point_t dvfs_op) {
    int ms = 0;
    int retval = 0;

    switch (dvfs_op) {
    case CORE_TURBO:
		//514 MHz
        __raw_writel(__raw_readl(MXC_CCM_MCR) | MXC_CCM_MCR_TPE, MXC_CCM_MCR);
        __raw_writel(0x00194817, MXC_CCM_TPCTL);
        SetArgonLVTurboMode(MAX_PDF_4);
        break;
    case CORE_TURBO_4:
		//680 MHz
        __raw_writel(__raw_readl(MXC_CCM_MCR) | MXC_CCM_MCR_TPE, MXC_CCM_MCR);
        __raw_writel((13 << MXC_CCM_TPCTL_MFI_OFFSET) |
                     (1 << MXC_CCM_TPCTL_MFN_OFFSET) |
                     (12 << MXC_CCM_PCTL_MFD_OFFSET) |
                     (0 << MXC_CCM_PCTL_PDF_OFFSET), MXC_CCM_TPCTL);
        SetArgonLVTurboMode(MAX_PDF_5);
        break;
    case CORE_TURBO_3:
		//642.5 MHz
        __raw_writel(__raw_readl(MXC_CCM_MCR) | MXC_CCM_MCR_TPE, MXC_CCM_MCR);
        __raw_writel((12 << MXC_CCM_TPCTL_MFI_OFFSET) |
                     (71 << MXC_CCM_TPCTL_MFN_OFFSET) |
                     (199 << MXC_CCM_PCTL_MFD_OFFSET) |
                     (0 << MXC_CCM_PCTL_PDF_OFFSET), MXC_CCM_TPCTL);
        SetArgonLVTurboMode(MAX_PDF_5);
        break;
    case CORE_TURBO_2:
		//548 MHz
        __raw_writel(__raw_readl(MXC_CCM_MCR) | MXC_CCM_MCR_TPE, MXC_CCM_MCR);
        __raw_writel((10 << MXC_CCM_TPCTL_MFI_OFFSET) |
                     (7 << MXC_CCM_TPCTL_MFN_OFFSET) |
                     (12 << MXC_CCM_PCTL_MFD_OFFSET) |
                     (0 << MXC_CCM_PCTL_PDF_OFFSET), MXC_CCM_TPCTL);
        SetArgonLVTurboMode(MAX_PDF_4);
        break;
    case CORE_TURBO_1:
		//532 MHz
        __raw_writel((10 << MXC_CCM_TPCTL_MFI_OFFSET) |
                     (3 << MXC_CCM_TPCTL_MFN_OFFSET) |
                     (12 << MXC_CCM_PCTL_MFD_OFFSET) |
                     (0 << MXC_CCM_PCTL_PDF_OFFSET), MXC_CCM_TPCTL);
        SetArgonLVTurboMode(MAX_PDF_4);
        break;
    case CORE_NORMAL:
		//385 MHz
        if (mxc_pm_chk_tpsel() == TPSEL_SET) {
            retval = mxc_pm_setturbo(CORE_TURBO, MAX_PDF_3);
            if (retval == ERR_DFSP_SWITCH) {
                return -ERR_DFSP_SWITCH;
            }
            while ((__raw_readl(MXC_CCM_MCR) & MXC_CCM_MCR_MPL) != MXC_CCM_MCR_MPL);
            while (ms < 5000) {
                if (((__raw_readl(MXC_CCM_PMCR0) >> 24) & 1) == 1) {
                    break;
                }
                suSleep(1, NULL);
                ms++;
            }
            if (ms >= 5000 && mxc_pm_chk_dfsp() == DFS_SWITCH) {
                return -ERR_DFSP_SWITCH;
            }
            __raw_writel(__raw_readl(MXC_CCM_PMCR0) | MXC_CCM_PMCR0_DFSI, MXC_CCM_PMCR0);
            __raw_writel(__raw_readl(MXC_CCM_MCR) & ~MXC_CCM_MCR_TPE, MXC_CCM_MCR);
        } else {
            mxc_pm_setbrmm(MPDR0_BRMM_0);
        }
        break;
    }

	LOG("MCUPLL:\t\t%lu\n", mxc_pll_clock(MCUPLL));
	LOG("TURBOPLL:\t\t%lu\n", mxc_pll_clock(TURBOPLL));
	LOG("USBPLL:\t\t%lu\n", mxc_pll_clock(USBPLL));
	LOG("CPU_CLK:\t%lu\n", mxc_get_clocks(CPU_CLK));
	LOG("AHB_CLK:\t%lu\n", mxc_get_clocks(AHB_CLK));
	LOG("IPG_CLK:\t%lu\n", mxc_get_clocks(IPG_CLK));
	LOG("NFC_CLK:\t%lu\n", mxc_get_clocks(NFC_CLK));
	LOG("USB_CLK:\t%lu\n", mxc_get_clocks(USB_CLK));
	LOG("UART1_BAUD:\t%lu\n", mxc_get_clocks(UART1_BAUD));
	LOG("UART2_BAUD:\t%lu\n", mxc_get_clocks(UART2_BAUD));
	LOG("UART3_BAUD:\t%lu\n", mxc_get_clocks(UART3_BAUD));
	LOG("I2C_CLK:\t%lu\n", mxc_get_clocks(I2C_CLK));
	LOG("IPU_CLK:\t%lu\n", mxc_get_clocks(IPU_CLK));
	LOG("SDMA_CLK:\t%lu\n", mxc_get_clocks(SDMA_CLK));
	LOG("SDHC1_CLK:\t%lu\n", mxc_get_clocks(SDHC1_CLK));
	LOG("SDHC2_CLK:\t%lu\n", mxc_get_clocks(SDHC2_CLK));

    return 0;
}

unsigned long mxc_mcu_active_pll_clk(void) {
    return (__raw_readl(MXC_CCM_MPDR0) & MXC_CCM_MPDR0_TPSEL) ? mxc_pll_clock(TURBOPLL) : mxc_pll_clock(MCUPLL);
}

unsigned long mxc_get_clocks(enum mxc_clocks clk) {
    unsigned long pll, ret_val = 0;
    unsigned long brmm, max_pdf, ipg_pdf, nfc_pdf, prepdf = 0, pdf = 0;
    int tpsel = 0;
    unsigned long reg1 = __raw_readl(MXC_CCM_MPDR0);
    unsigned long reg2 = __raw_readl(MXC_CCM_MPDR1);
    unsigned long reg3 = __raw_readl(MXC_CCM_MPDR2);

    max_pdf = (reg1 & MXC_CCM_MPDR0_MAX_PDF_MASK) >> MXC_CCM_MPDR0_MAX_PDF_OFFSET;
    ipg_pdf = (reg1 & MXC_CCM_MPDR0_IPG_PDF_MASK) >> MXC_CCM_MPDR0_IPG_PDF_OFFSET;
    pll = mxc_mcu_active_pll_clk();

    switch (clk) {
    case CPU_CLK:
        brmm = reg1 & MXC_CCM_MPDR0_BRMM_MASK;
        tpsel = (reg1 & MXC_CCM_MPDR0_TPSEL) >> MXC_CCM_MPDR0_TPSEL_OFFSET;
        if (brmm >= 5) {
            return 0;
        }
        ret_val = mxc_get_clocks(AHB_CLK) == AHB_FREQ_MAX ? core_pll_op_128[tpsel][brmm] : core_pll_op_100[tpsel][brmm];
        break;
    case AHB_CLK:
    case IPU_CLK:
        ret_val = pll / (max_pdf + 1);
        break;
    case IPG_CLK:
    case SIM1_CLK:
    case SIM2_CLK:
    case UART1_BAUD:
    case UART2_BAUD:
    case UART3_BAUD:
    case UART4_BAUD:
    case OWIRE_CLK:
    case I2C_CLK:
        ret_val = pll / ((max_pdf + 1) * (ipg_pdf + 1));
        break;
    case GPT_CLK:
        ret_val = MXC_TIMER_CLK;
        break;
    case SDHC1_CLK:
        if (reg3 & MXC_CCM_MPDR2_SDHC1DIS) {
            return 0;
        }
        pll = mxc_get_clocks_parent(clk);
        pdf = (reg3 & MXC_CCM_MPDR2_SDHC1_PDF_MASK) >> MXC_CCM_MPDR2_SDHC1_PDF_OFFSET;
        ret_val = pll / (pdf + 1);
        break;
    case SDHC2_CLK:
        if (reg3 & MXC_CCM_MPDR2_SDHC2DIS) {
            return 0;
        }
        pll = mxc_get_clocks_parent(clk);
        pdf = (reg3 & MXC_CCM_MPDR2_SDHC2_PDF_MASK) >> MXC_CCM_MPDR2_SDHC2_PDF_OFFSET;
        ret_val = pll / (pdf + 1);
        break;
    case NFC_CLK:
        nfc_pdf = (reg1 & MXC_CCM_MPDR0_NFC_PDF_MASK) >> MXC_CCM_MPDR0_NFC_PDF_OFFSET;
        ret_val = pll / ((max_pdf + 1) * (nfc_pdf + 1));
        break;
    case USB_CLK:
        if (reg2 & MXC_CCM_MPDR1_USB_DIS) {
            return 0;
        }
        pdf = (reg2 & MXC_CCM_MPDR1_USB_PDF_MASK) >> MXC_CCM_MPDR1_USB_PDF_OFFSET;
        pll = mxc_pll_clock(USBPLL);
        ret_val = pll / (pdf + 1);
        break;
    case SSI1_BAUD:
        if (reg2 & MXC_CCM_MPDR1_SSI1_DIS) {
            return 0;
        }
        prepdf = (reg2 & MXC_CCM_MPDR1_SSI1_PREPDF_MASK) >> MXC_CCM_MPDR1_SSI1_PREPDF_OFFSET;
        pdf = (reg2 & MXC_CCM_MPDR1_SSI1_PDF_MASK) >> MXC_CCM_MPDR1_SSI1_PDF_OFFSET;
        pll = mxc_get_clocks_parent(clk);
        ret_val = pll / ((prepdf + 1) * (pdf + 1));
        break;
    case SSI2_BAUD:
        if (reg2 & MXC_CCM_MPDR1_SSI2_DIS) {
            return 0;
        }
        prepdf = (reg2 & MXC_CCM_MPDR1_SSI2_PREPDF_MASK) >> MXC_CCM_MPDR1_SSI2_PREPDF_OFFSET;
        pdf = (reg2 & MXC_CCM_MPDR1_SSI2_PDF_MASK) >> MXC_CCM_MPDR1_SSI2_PDF_OFFSET;
        pll = mxc_get_clocks_parent(clk);
        ret_val = pll / ((prepdf + 1) * (pdf + 1));
        break;
    case CSI_BAUD:
        pdf = (reg1 & MXC_CCM_MPDR0_CSI_PDF_MASK) >> MXC_CCM_MPDR0_CSI_PDF_OFFSET;
        if (reg1 & MXC_CCM_MPDR0_CSI_DIS) {
            return 0;
        }
        pll = mxc_get_clocks_parent(clk);
        prepdf = (reg1 & MXC_CCM_MPDR0_CSI_PRE) ? 2 : 1;
        ret_val = (reg1 & MXC_CCM_MPDR0_CSI_FPDF) ?
                  (2 * pll) / (prepdf * (2 * pdf + 3)) :
                  pll / (prepdf * (pdf + 1));
        break;
    case FIRI_BAUD:
        if (reg2 & MXC_CCM_MPDR1_FIRI_DIS) {
            return 0;
        }
        prepdf = (reg2 & MXC_CCM_MPDR1_FIRI_PREPDF_MASK) >> MXC_CCM_MPDR1_FIRI_PREPDF_OFFSET;
        pdf = (reg2 & MXC_CCM_MPDR1_FIRI_PDF_MASK) >> MXC_CCM_MPDR1_FIRI_PDF_OFFSET;
        pll = mxc_get_clocks_parent(clk);
        ret_val = pll / ((prepdf + 1) * (pdf + 1));
        break;
    case CKIH_CLK:
        ret_val = MXC_CKIH_FREQ;
        break;
	default:
		break;
    }
    return ret_val;
}
#else
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
#define DPLL_DIVIDE_BY_5          0x3
#define DPLL_DIVIDE_BY_6          0x4
#define DPLL_DIVIDE_BY_8          0x5
#define DPLL_DIVIDE_BY_10         0x6
#define DPLL_DIVIDE_BY_12         0x7

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

#if defined(DISABLE_ALL_INT)
	UINT32 int_mask = suDisableAllInt();
#endif

	status = RESULT_OK;
	dpll_clock_rate = 0;
	dpll_reg_value_tbl_ptr = NULL;

	switch (neptune_clock) {
#if 0 /* Original code. */
		case CLOCKS_13MHZ_26MHZ: /* 13000000 hz  - 1410 - 83  */
			*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_13_MHZ;
			hapi_clock_reg->clk_sel &= ~MCU_CLK_SEL_MASK;
			hapi_clock_reg->clk_sel &= ~REF_SEL_MASK;
			hapi_clock_reg->clk_sel |= CODEC_CLK_SEL_MASK;
			break;
#endif
		case CLOCKS_13MHZ_13MHZ: /* UNKNOWN */
			*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_13_MHZ;
			dpll_clock_rate = DPLL_CLOCK_RATE_39_MHZ; /* 156 MHz */
			hapi_clock_reg->div_factor = /* 0x0700 (156 / 12) = 13 MHz */
				(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | (DPLL_DIVIDE_BY_12 << REF_DPLL_DIV_SHIFT);
			hapi_clock_reg->div_factor = /* 0x0007 (156 / 12) = 13 MHz */
				(hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | (DPLL_DIVIDE_BY_12 << MCU_DPLL_DIV_SHIFT);
			break;
		case CLOCKS_13MHZ_26MHZ: /* 13000000 hz  - 1410 - 83  */
			*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_13_MHZ;
			dpll_clock_rate = DPLL_CLOCK_RATE_39_MHZ; /* 156 MHz */
			hapi_clock_reg->div_factor = /* 0x0700  (156 / 6) = 26 MHz */
				(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | (DPLL_DIVIDE_BY_6  << REF_DPLL_DIV_SHIFT);
			hapi_clock_reg->div_factor = /* 0x0004 (156 / 12) = 13 MHz */
				(hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | (DPLL_DIVIDE_BY_12 << MCU_DPLL_DIV_SHIFT);
			break;
		case CLOCKS_26MHZ_21MHZ: /* UNKNOWN */
			*hapi_clock_rate_mcu = HAPI_CLOCK_RATES_26_MHZ;
			dpll_clock_rate = DPLL_CLOCK_RATE_26_MHZ; /* 260 MHz */
			hapi_clock_reg->div_factor = /* 0x0700 (260 / 12) = 21.3(3) MHz */
				(hapi_clock_reg->div_factor & ~REF_DPLL_DIV_MASK) | (DPLL_DIVIDE_BY_12 << REF_DPLL_DIV_SHIFT);
			hapi_clock_reg->div_factor = /* 0x0006 (260 / 10) = 26 MHz      */
				(hapi_clock_reg->div_factor & ~MCU_DPLL_DIV_MASK) | (MCD_DIVIDE_26_MHZ << MCU_DPLL_DIV_SHIFT);
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

#if defined(DISABLE_ALL_INT)
	suSetInt(int_mask);
#endif

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

#endif