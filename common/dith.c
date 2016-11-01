#include <common.h>
#include <asm/io.h>
#include <ids/ids.h>

void dith_reduce_rgb(int rchan, int gchan, int bchan)
{
	uint32_t val;

	val = readl(reg_base + DIT_DCR);
	val &= ~(0x1ff << DIT_DCR_BNB);
	val |= (((rchan & 0x7) << DIT_DCR_RNB) | ((gchan & 0x7) << DIT_DCR_GNB) |
		    ((bchan & 0x7) << DIT_DCR_BNB));
	writel(val, reg_base + DIT_DCR);
}

void dith_set_coefficient(int value, enum dit_dmco coefficient)
{
	int inter, fract;
	uint32_t val;
	
	fract = coefficient / 4;
	inter = coefficient % 4;
	inter = 8 * (3 - inter);

	val = readl(DIT_DMCO(fract));
	val &= ~(0xf << inter);
	val |= ((value & 0xf) << inter);
	writel(val, DIT_DMCO(fract));	
}

void dith_set_size(int width, int height)
{
	uint32_t val;

	val = (((width - 1) & 0xfff) << 16) | ((height - 1) & 0xfff);
	writel(val, reg_base + DIT_FSR);
}

void dith_ctrl_temporal(int enable)
{
	uint32_t val;

	val = readl(reg_base + DIT_DCR);
	if (enable)
		val |= 0x1;
	else
		val &= ~0x1;
	writel(val, reg_base + DIT_DCR);
}

void dith_ctrl_dither(int enable)
{
	uint32_t val;

	val = readl(reg_base + DIT_DCR);
	if (enable)
		val &= ~(0x1 << DIT_DCR_PASSBY);
	else
		val |= (0x1 << DIT_DCR_PASSBY);
	writel(val, reg_base + DIT_DCR);
}

void dith_init(void)
{
	dith_ctrl_temporal(DISABLE);
	dith_ctrl_dither(DISABLE);	
}
