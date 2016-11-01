#include <common.h>
#include <cvbs.h>
#include <ids/ids.h>
#include <irq.h>

//static int32_t YCbCr2RGB[] = {16, 0, 596, 0, 817, 596, 3895, 3680, 596, 1033, 0};
//static int32_t RGB2YCbCr[] = {0, 16, 66, 129, 25, -38, -74, 112, 112, -94, -18};   // -76, -149, -188, -37a
//static int32_t RGB2YCbCr[] = {0, 16, 131, 258, 50, -38, -74, 225, 225, -94, -18};
//static int32_t RGB2YCbCr[] = {0, 16, 131, 258, 50, -38, -74, 225, 225, -94, -18};
static int32_t RGB2YCbCr[] = {0, 16, 66, 129, 25, 2010, 1974, 112, 112, 1954, 2030};
static int len_cfg[][2][8] = {
	[0] = {
		{ 22, 576, 2, 23, 576, 2, 280, 1440 },
		{ 19, 480, 3, 20, 480, 3, 268, 1440 },
	}, 
	[1] = {
		{ 22, 576, 2, 23, 576, 2, 136, 720 },
		{ 19, 480, 3, 20, 480, 3, 130, 720 },
	},
	[2] = {
		{ 28, 720, 12, 28, 720, 12, 125, 576 },
		{ 28, 720, 12, 28, 720, 12, 125, 480 },
	},
	[3] = {
		{ 22, 288, 2, 23, 288, 2, 280, 1440 },
		{ 19, 240, 3, 20, 240, 3, 268, 1440 },
	}, 
	[4] = {
		{ 22, 288, 2, 23, 288, 2, 136, 720 },
		{ 19, 240, 3, 20, 240, 3, 130, 720 },
	},
	[5] = {
		{ 28, 720, 12, 28, 720, 12, 125, 288 },
		{ 28, 720, 12, 28, 720, 12, 125, 240 },
	},
};

static int sync_cfg[][2][7] = {
	[0] = {
		{ 0, 24, 126, 24, 4320, 888, 4320 },
		{ 0, 32, 124, 32, 5148, 890, 5148 },
	},
	[1] = {
		{ 0, 12, 63, 0, 2160, 432, 2160 },
		{ 0, 16, 62, 0, 2574, 429, 2574 },
	},
	[2] = {
		{ 10, 10, 10, 10, 10, 10, 10 },
		{ 10, 10, 10, 10, 10, 10, 10 },
	},
};

static int display[][2] = {
	{ 720, 576 },
	{ 720, 480 },
};

static struct reg_check reg[] = {
	REG_CHECK_INIT(TVIF_CLK_CFG_ADDR,		0x80000FFF),
	REG_CHECK_INIT(TVIF_CFG_ADDR,			0xF0073C01),
	REG_CHECK_INIT(TVIF_COEF11_ADDR,		0xffffffff),
	REG_CHECK_INIT(TVIF_COEF12_ADDR,		0xffffffff),
	REG_CHECK_INIT(TVIF_COEF13_ADDR,		0xffffffff),
	REG_CHECK_INIT(TVIF_COEF21_ADDR,		0xffffffff),
	REG_CHECK_INIT(TVIF_COEF22_ADDR,		0xffffffff),
	REG_CHECK_INIT(TVIF_COEF23_ADDR,		0xffffffff),
	REG_CHECK_INIT(TVIF_COEF31_ADDR,		0xffffffff),
	REG_CHECK_INIT(TVIF_COEF32_ADDR,		0xffffffff),
	REG_CHECK_INIT(TVIF_COEF33_ADDR,		0xffffffff),
	REG_CHECK_INIT(TVIF_MATRIX_CFG_ADDR,		0xf0001f1f),
	REG_CHECK_INIT(TVIF_UBA1_LEN_ADDR,		0x000007ff),
	REG_CHECK_INIT(TVIF_UNBA_LEN_ADDR,		0x000007ff),
	REG_CHECK_INIT(TVIF_UBA2_LEN_ADDR,		0x000007ff),
	REG_CHECK_INIT(TVIF_LBA1_LEN_ADDR,		0x000007ff),
	REG_CHECK_INIT(TVIF_LNBA_LEN_ADDR,		0x000007ff),
	REG_CHECK_INIT(TVIF_LBA2_LEN_ADDR,		0x000007ff),
	REG_CHECK_INIT(TVIF_BLANK_LEN_ADDR,		0x00000fff),
	REG_CHECK_INIT(TVIF_VIDEO_LEN_ADDR,		0x00000fff),
	REG_CHECK_INIT(TVIF_HSYNC_ADDR,			0xdfff1fff),
	REG_CHECK_INIT(TVIF_VSYNC_UPPER_ADDR,		0x1fff1fff),
	REG_CHECK_INIT(TVIF_VSYNC_LOWER_ADDR,		0x1fff1fff),
	REG_CHECK_INIT(TVIF_DISP_XSIZE_ADDR,		0x000007ff),
	REG_CHECK_INIT(TVIF_DISP_YSIZE_ADDR,		0x000007ff),
	REG_CHECK_INIT(TVIF_STATUS_ADDR,		0x00000000),
};

int tvif_reg_check(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(reg); i++)
		reg[i].addr += reg_base;

	if (common_reg_check(reg, ARRAY_SIZE(reg), 0xffffffff) || common_reg_check(reg, ARRAY_SIZE(reg), 0))
		return -1;

	return 0;
}

static void tvif_writel(uint32_t offset, uint32_t data,
					uint8_t shift, uint8_t width)
{
	uint32_t mask = (1 << width) - 1;
	uint32_t val;

	val = readl(reg_base + offset);
	val &= ~(mask << shift);
	val |= (data & mask) << shift;
	writel(val, reg_base + offset);
}

static uint32_t tvif_readl(uint32_t offset, uint8_t shift, uint8_t width)
{
	uint32_t mask = (1 << width) - 1;
	uint32_t val;

	val = readl(reg_base + offset);
	val &= (mask << shift);
	val >>= shift;

	return val;
}

void tvif_set_matrix(struct cvbs_matrix *param)
{
	if (!param)
		return;
	
	if (param->bypass) {
		tvif_writel(TVIF_MATRIX_CFG_ADDR, param->bypass, TVIF_MATRIX_CFG_PASSBY, 1);
		return;
	}

	tvif_writel(TVIF_MATRIX_CFG_ADDR, param->mode, TVIF_MATRIX_CFG_toRGB, 1);
	tvif_writel(TVIF_MATRIX_CFG_ADDR, param->inv_MSB_in, TVIF_MATRIX_CFG_INV_MSB_IN, 1);
	tvif_writel(TVIF_MATRIX_CFG_ADDR, param->inv_MSB_out, TVIF_MATRIX_CFG_INV_MSB_OUT, 1);
	if (param->mode == MATRIX_RGB2YUV) {
		tvif_writel(TVIF_MATRIX_CFG_ADDR, param->coef.out_oft_b, TVIF_MATRIX_CFG_OFT_B, 5);
		tvif_writel(TVIF_MATRIX_CFG_ADDR, param->coef.out_oft_a, TVIF_MATRIX_CFG_OFT_A, 5);
		tvif_writel(TVIF_COEF11_ADDR, param->coef.out_coef11, 0, 32);
		tvif_writel(TVIF_COEF12_ADDR, param->coef.out_coef12, 0, 32);
		tvif_writel(TVIF_COEF13_ADDR, param->coef.out_coef13, 0, 32);
		tvif_writel(TVIF_COEF21_ADDR, param->coef.out_coef21, 0, 32);
		tvif_writel(TVIF_COEF22_ADDR, param->coef.out_coef22, 0, 32);
		tvif_writel(TVIF_COEF23_ADDR, param->coef.out_coef23, 0, 32);
		tvif_writel(TVIF_COEF31_ADDR, param->coef.out_coef31, 0, 32);
		tvif_writel(TVIF_COEF32_ADDR, param->coef.out_coef32, 0, 32);
		tvif_writel(TVIF_COEF33_ADDR, param->coef.out_coef33, 0, 32);
	}
}

void tvif_set_len_cfg(struct cvbs_len_cfg *cfg)
{
	if (!cfg)
		return;
#if 0
	printf("tvif_set_len_cfg\n");
	printf("%d\n", cfg->uba1_len);
	printf("%d\n", cfg->unba_len);
	printf("%d\n", cfg->uba2_len);
	printf("%d\n", cfg->lba1_len);
	printf("%d\n", cfg->lnba_len);
	printf("%d\n", cfg->lba2_len);
	printf("%d\n", cfg->blank_len);
	printf("%d\n", cfg->video_len);
#endif
	tvif_writel(TVIF_UBA1_LEN_ADDR, cfg->uba1_len, 0, 11);
	tvif_writel(TVIF_UNBA_LEN_ADDR, cfg->unba_len, 0, 11);
	tvif_writel(TVIF_UBA2_LEN_ADDR, cfg->uba2_len, 0, 11);
	tvif_writel(TVIF_LBA1_LEN_ADDR, cfg->lba1_len, 0, 11);
	tvif_writel(TVIF_LNBA_LEN_ADDR, cfg->lnba_len, 0, 11);
	tvif_writel(TVIF_LBA2_LEN_ADDR, cfg->lba2_len, 0, 11);
	tvif_writel(TVIF_BLANK_LEN_ADDR, cfg->blank_len, 0, 12);
	tvif_writel(TVIF_VIDEO_LEN_ADDR, cfg->video_len, 0, 12);
}

void tvif_set_sync_cfg(struct cvbs_sync_cfg *cfg)
{
	if (!cfg)
		return;
#if 0
	printf("tvif_set_sync_cfg\n");
	printf("%d\n", cfg->Hsync_VBI_ctrl);
	printf("%d\n", cfg->Hsync_delay);
	printf("%d\n", cfg->Hsync_extend);
	printf("%d\n", cfg->Vsync_delay_upper);
	printf("%d\n", cfg->Vsync_extend_upper);
	printf("%d\n", cfg->Vsync_delay_lower);
	printf("%d\n", cfg->Vsync_extend_lower);
#endif
	tvif_writel(TVIF_HSYNC_ADDR, cfg->Hsync_VBI_ctrl, TVIF_HSYNC_VBI_CTRL, 2);
	tvif_writel(TVIF_HSYNC_ADDR, cfg->Hsync_delay, TVIF_HSYNC_DELAY, 13);
	tvif_writel(TVIF_HSYNC_ADDR, cfg->Hsync_extend, TVIF_HSYNC_EXTEND, 13);
	tvif_writel(TVIF_VSYNC_UPPER_ADDR, cfg->Vsync_delay_upper, TVIF_VSYNC_DELAY, 13);
	tvif_writel(TVIF_VSYNC_UPPER_ADDR, cfg->Vsync_extend_upper, TVIF_VSYNC_EXTEND, 16);
	tvif_writel(TVIF_VSYNC_LOWER_ADDR, cfg->Vsync_delay_lower, TVIF_VSYNC_DELAY, 13);
	tvif_writel(TVIF_VSYNC_LOWER_ADDR, cfg->Vsync_extend_lower, TVIF_VSYNC_EXTEND, 16);
}

void tvif_set_display(struct cvbs_display *cfg)
{
	if (!cfg)
		return;

	tvif_writel(TVIF_DISP_XSIZE_ADDR, cfg->xsize - 1, 0, 11);
	tvif_writel(TVIF_DISP_YSIZE_ADDR, cfg->ysize - 1, 0, 11);
}

void tvif_get_display(int *xsize, int *ysize)
{
	*xsize = tvif_readl(TVIF_DISP_XSIZE_ADDR, 0, 11) + 1;
	*ysize = tvif_readl(TVIF_DISP_YSIZE_ADDR, 0, 11) + 1;
}

void tvif_set_clk(int num)
{
	uint32_t val;
	int clk;
	
	if (num)
		clk = module_get_clock("ids1-eitf");
	else
		clk = module_get_clock("ids0-eitf");

#ifndef CONFIG_COMPILE_FPGA
	val = (1 << TVIF_CLK_CFG_EN_CLOCK) | (0 << TVIF_CLK_CFG_PB_CLOCK) |
		  (0 << TVIF_CLK_CFG_INV_CLOCK) | (1 << TVIF_CLK_CFG_SEL_CLOCK) |
		  ((clk / 27000000 - 1) << TVIF_CLK_CFG_DIVER_CLOCK);
#else
	val = (1 << TVIF_CLK_CFG_EN_CLOCK) | (0 << TVIF_CLK_CFG_PB_CLOCK) |
		  (0 << TVIF_CLK_CFG_INV_CLOCK) | (1 << TVIF_CLK_CFG_SEL_CLOCK) |
		  (0 << TVIF_CLK_CFG_DIVER_CLOCK);
#endif
	tvif_writel(TVIF_CLK_CFG_ADDR, val, 0, 32);
}

void tvif_set_cfg(struct cvbs_cfg *cfg)
{
	uint32_t val;

	val = (cfg->itu601_656n << TVIF_CFG_ITU601_656n) |
		  (cfg->Bit16ofITU60 << TVIF_CFG_BIT16ofITU601) |
		  (cfg->Direct_data << TVIF_CFG_DIRECT_DATA) |
		  (cfg->bitsSwap << TVIF_CFG_BITSWAP) |
		  (cfg->dataOrder << TVIF_CFG_DATA_ORDER) |    
		  (cfg->invVsync << TVIF_CFG_INV_VSYNC) |    
		  (cfg->invHsync << TVIF_CFG_INV_HSYNC) |    
		  (cfg->invHref << TVIF_CFG_INV_HREF) |    
		  (cfg->invField << TVIF_CFG_INV_FIELD) |    
		  (cfg->beginEAV << TVIF_CFG_Begin_by_EAV);
	tvif_writel(TVIF_CFG_ADDR, val, 0, 32);
}

void tvif_ctrl(int flags)
{
	tvif_writel(TVIF_CFG_ADDR, flags, TVIF_CFG_TVIF_EN, 1);
}

int tvif_frame_irq(void)
{
	uint32_t status;
	int ret = -1;

	status = readl(reg_base + IDSINTPND);
	if (status & (1 << IDSINTPND_TVIFINT)) {
		ids_dbg("success to send a tvif frame\n");
		ret = 0;
		irq_num++;
	}
	writel(status, reg_base + IDSSRCPND);
	writel(status, reg_base + IDSINTPND);

	return ret;	
}

void tvif_wait_frame(void)
{
	while (tvif_frame_irq())
		;
}

void tvif_irq_mode(void)
{
	int ret;

	ret = request_irq(IDS0_INT_ID, (irq_handler_t)tvif_frame_irq, "tvif");
	if (ret < 0) {
		ids_err("Tvif request irq err\n");
		return;
	}	
}

static void tvif_default_cfg(int flags, int mode, int bit, enum TVIF_MODE interface)
{
	struct cvbs_default_cfg def;
	

	if ((flags != f_PAL) && (flags != f_NTSC))
		return;

	def.cfg.itu601_656n = mode;
	def.cfg.Bit16ofITU60 = bit;
	if (bit == CVBS_24BIT)
		def.cfg.Direct_data = CVBS_RGB;
	else
		def.cfg.Direct_data = CVBS_YUV;
	def.cfg.bitsSwap = DISABLE;
	if (bit == CVBS_24BIT)
		def.cfg.dataOrder = 0;
	else
		def.cfg.dataOrder = 1;
	if (mode == CVBS_ITU601)
		def.cfg.invVsync = ENABLE;
	else
		def.cfg.invVsync = DISABLE;
	def.cfg.invHsync = DISABLE; 
	def.cfg.invHref = DISABLE;
	def.cfg.invField = DISABLE;
	def.cfg.beginEAV = LINE_WITH_EAV;
	def.param.mode = MATRIX_RGB2YUV;
	if (bit == CVBS_24BIT)
		def.param.bypass = 1;
	else
		def.param.bypass = 0;
	def.param.inv_MSB_in = DISABLE;
	def.param.inv_MSB_out = DISABLE;
	memcpy(&(def.param.coef), RGB2YCbCr, 4 * ARRAY_SIZE(RGB2YCbCr));
	memcpy(&(def.display), display[flags], 4 * ARRAY_SIZE(display[flags]));
	memcpy(&(def.sync_cfg), sync_cfg[bit][flags], 4 * ARRAY_SIZE(sync_cfg[bit][flags]));
	memcpy(&(def.len_cfg), len_cfg[bit + 3 * interface][flags], 4 * ARRAY_SIZE(len_cfg[bit + 3 * interface][flags]));

	tvif_set_display(&(def.display));
	tvif_set_clk(0);
	tvif_set_cfg(&(def.cfg));
	tvif_set_matrix(&(def.param));
	tvif_set_len_cfg(&(def.len_cfg));
	tvif_set_sync_cfg(&(def.sync_cfg));
}

int tvif_close(void)
{
	return cvbs_close();
}

int tvif_cfg(int flags, int mode, int bit, enum TVIF_MODE interface)
{
	tvif_default_cfg(flags, mode, bit, interface);
	return cvbs_dev_cfg(flags);
}

int tvif_open(char *name)
{
	int ret;

	ret = cvbs_open(name);
	if (ret < 0) {
		cvbs_err("Tvif: cannot find %s cvbs chip\n", name);
		return -1;
	}

	return 0;
}

int tvif_init(int num)
{
	lcd_set_output_if(TV_DISPLAY);	
	if (num) {
		module_set_clock("ids1-eitf", "epll", 54000000, 0);
	} else {
		module_set_clock("ids0-eitf", "epll", 54000000, 0);
	}
	return 0;
}
