#include <common.h>
#include <asm/io.h>
#include <ids/ids.h>
#include <dmmu.h>

static struct reg_check reg[] = {
	REG_CHECK_INIT(OVCDCR,			0x00407FFF),
	REG_CHECK_INIT(OVCPCR,			0x0000803F),
	REG_CHECK_INIT(OVCBKCOLOR,		0x00FFFFFF),
	REG_CHECK_INIT(OVCWPR,			0x00FFFFFF),
	REG_CHECK_INIT(OVCW0CR,			0x0007FF7F),
	REG_CHECK_INIT(OVCW0PCAR,		0x0FFF0FFF),
	REG_CHECK_INIT(OVCW0PCBR,		0x0FFF0FFF),
	REG_CHECK_INIT(OVCW0VSSR,		0x77FFFFFF),
	REG_CHECK_INIT(OVCW0CMR,		0x01FFFFFF),
	REG_CHECK_INIT(OVCW0B0SAR,		0xFFFFFFFF),
	REG_CHECK_INIT(OVCW0B1SAR,		0xFFFFFFFF),
	REG_CHECK_INIT(OVCW0B2SAR,		0xFFFFFFFF),
	REG_CHECK_INIT(OVCW0B3SAR,		0xFFFFFFFF),
	REG_CHECK_INIT(OVCW1CR,			0x0007FFFF),
	REG_CHECK_INIT(OVCW1PCAR,		0x0FFF0FFF),
	REG_CHECK_INIT(OVCW1PCBR,		0x0FFF0FFF),
	REG_CHECK_INIT(OVCW1PCCR,		0x00FFFFFF),
	REG_CHECK_INIT(OVCW1VSSR,		0x77FFFFFF),
	REG_CHECK_INIT(OVCW1CKCR,		0x07FFFFFF),
	REG_CHECK_INIT(OVCW1CKR,		0x00FFFFFF),
	REG_CHECK_INIT(OVCW1CMR,		0x01FFFFFF),
	REG_CHECK_INIT(OVCW1B0SAR,		0xFFFFFFFF),
	REG_CHECK_INIT(OVCW1B1SAR,		0xFFFFFFFF),
	REG_CHECK_INIT(OVCW1B2SAR,		0xFFFFFFFF),
	REG_CHECK_INIT(OVCW1B3SAR,		0xFFFFFFFF),
	REG_CHECK_INIT(OVCBRB0SAR,		0xFFFFFFFF),
	REG_CHECK_INIT(OVCBRB1SAR,      0xFFFFFFFF),
	REG_CHECK_INIT(OVCBRB2SAR,      0xFFFFFFFF),	
	REG_CHECK_INIT(OVCBRB3SAR,      0xFFFFFFFF),
	REG_CHECK_INIT(OVCOMC,			0x811FFFFF),
	REG_CHECK_INIT(OVCOEF11,		0x000007FF),
	REG_CHECK_INIT(OVCOEF12,		0x000007FF),
	REG_CHECK_INIT(CICOEF13,		0x000007FF),
	REG_CHECK_INIT(OVCOEF21,		0x000007FF),
	REG_CHECK_INIT(OVCOEF22,		0x000007FF),
	REG_CHECK_INIT(OVCOEF23,		0x000007FF),
	REG_CHECK_INIT(OVCOEF31,		0x000007FF),
	REG_CHECK_INIT(OVCOEF32,		0x000007FF),
	REG_CHECK_INIT(OVCOEF33,		0x000007FF),
	REG_CHECK_INIT(OVCPSCCR,		0x1FFFDFFF),
	REG_CHECK_INIT(OVCPSCPCR,		0x07FF07FF),
	REG_CHECK_INIT(OVCPSVSSR,		0x77FFFFFF),
	REG_CHECK_INIT(OVCPSB0SAR,		0xFFFFFFFF),
	REG_CHECK_INIT(OVCPSB1SAR,		0xFFFFFFFF),
	REG_CHECK_INIT(OVCPSB2SAR,		0xFFFFFFFF),
	REG_CHECK_INIT(OVCPSB3SAR,		0xFFFFFFFF),
	REG_CHECK_INIT(DIT_FSR,				0x0FFF0FFF),
	REG_CHECK_INIT(DIT_DCR,				0x00000FFD),
	REG_CHECK_INIT(DIT_DMCO(0),			0x1F1F1F1F),
	REG_CHECK_INIT(DIT_DMCO(1),         0x1F1F1F1F),
	REG_CHECK_INIT(DIT_DMCO(2),         0x1F1F1F1F),
	REG_CHECK_INIT(DIT_DMCO(3),         0x1F1F1F1F),
	REG_CHECK_INIT(DIT_DMCO(4),         0x1F1F1F1F),
	REG_CHECK_INIT(DIT_DMCO(5),         0x1F1F1F1F),
	REG_CHECK_INIT(DIT_DMCO(6),         0x1F1F1F1F),
	REG_CHECK_INIT(DIT_DMCO(7),         0x1F1F1F1F),
};

int osd_reg_check(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(reg); i++)
		reg[i].addr += reg_base;

	if (common_reg_check(reg, ARRAY_SIZE(reg), 0xffffffff) || common_reg_check(reg, ARRAY_SIZE(reg), 0))
		return -1;

	return 0;
}

void osd_set_dma_mode(int nr, struct osd_buf_mode *mode)
{
	uint32_t val, offset = OVCW0CR;
	
	if (nr)
		offset = OVCW1CR;

	val = readl(reg_base + offset);
	val &= ~(0x1f << OVCWxCR_BUFNUM);
	val |= (((mode->bufsel & 0x3) << OVCWxCR_BUFSEL) |
		   ((mode->bufauto & 0x1) << OVCWxCR_BUFAUTOEN) |
		   ((mode->bufnum & 0x3) << OVCWxCR_BUFNUM));
	writel(val, reg_base + offset);
}

static void osd_enable_ctrl(int nr, int enable)
{
	uint32_t val, offset = OVCW0CR;
	
	if (nr)
		offset = OVCW1CR;
	val = readl(reg_base + offset);
	if (enable)
		val |= (0x1 << OVCWxCR_ENWIN);
	else
		val &= ~(0x1 << OVCWxCR_ENWIN);
	writel(val, reg_base + offset);
}

static void osd_palette_ctrl(int enable)
{
	writel(enable << OVCPCR_UPDATE_PAL, reg_base + OVCPCR);
}

void osd_set_bg_color(uint32_t val)
{
	writel(val, reg_base + OVCBKCOLOR);
}

void osd_set_scale_xy(int x, int y)
{
	uint32_t val;

	val = ((x - 1) << OVCWPR_OSDWinX) |
		  ((y - 1) << OVCWPR_OSDWinY);
	writel(val, reg_base + OVCWPR);
}

static void osd_set_start_xy(int nr, int x, int y)
{
	uint32_t val, offset = OVCW0PCAR;
	
	if (nr)
		offset = OVCW1PCAR;

	val = (x << OVCWxPCAR_LeftTopX) |
		  (y << OVCWxPCAR_LeftTopY);
	writel(val, reg_base + offset);
}

void osd_set_end_xy(int nr, int x, int y)
{
	uint32_t val, offset = OVCW0PCBR;
	
	if (nr)
		offset = OVCW1PCBR;
	val = ((x - 1) << OVCWxPCBR_RightBotX) |
		  ((y - 1) << OVCWxPCBR_RightBotY);
	writel(val, reg_base + offset);
}

void osd_set_coord(int nr, struct osd_xy *coord)
{
	osd_set_start_xy(nr, coord->leftx, coord->lefty);
	osd_set_end_xy(nr, coord->rightx, coord->righty);	
}

void osd_set_vm_width(int nr, int val)
{
	uint32_t tmp, offset = OVCW0VSSR;

	if (nr)
		offset = OVCW1VSSR;
	tmp = val << OVCWVSSR_VW_WIDTH;
	writel(tmp, reg_base + offset);
}

void osd_set_yuvtorgb(struct osd_yuv *yuv)
{
	uint32_t val;
	int i;
	
	val = readl(reg_base + OVCOMC);
	val &= ~(0x1 << 31);
	val |= (yuv->enable << 31);
	writel(val, reg_base + OVCOMC);
	if (yuv->enable) {
		val = readl(reg_base + OVCOMC);
		val &= ~(0x1f << 0);
		val &= ~(0x1f << 8);
		val |= (yuv->out_oft_a) & 0x1f;
		val |= ((yuv->out_oft_b & 0x1f) << 8);
		writel(val, reg_base + OVCOMC);
		for (i = 0; i < 9; i++)
			writel(yuv->out_coef[i] & 0x7ff, OVCOEF11 + 4 * i + reg_base);
	}
}

void osd_colormap_ctrl(int nr, int enable, int color)
{
	uint32_t val, offset = OVCW0CMR;
	
	if (nr)
		offset = OVCW1CMR;

	val = (enable <<  OVCWCMRx_ENABLE) |
		  (color << OVCWCMRx_COLOR);
	writel(val, reg_base + offset);
}
void osd_set_changerb(int nr, int enable)
{       
        uint32_t offset = OVCW0CR;
        uint32_t val;
        
        if (nr)
                offset = OVCW1CR;
        
        val = readl(reg_base + offset);
        if (enable)
                val |= (0x1 << OVCWxCR_RBEXG);
        else
                val &= ~(0x1 << OVCWxCR_RBEXG);
        writel(val, reg_base + offset);
}

void osd_set_dma_channel(int nr, int num, struct dma_buffer *dma)
{
	uint32_t offset1 = OVCW0B0SAR;
	int i;
	
	if (num > 4)
		num = 4;

	if (num < 1 || num > 4) {
		ids_err("Window dma addr only support [1, 4]\n");
		return;
	}
	
	if (nr)
		offset1 = OVCW1B0SAR;

	for (i = 0; i < num;i++)
		writel((uint32_t)(dma[i].vir_addr), reg_base + offset1 + i * 4);
}

void osd_set_yuv_dma(int nr, int num, struct dma_buffer *dma)
{
	uint32_t offset1 = OVCW0B0SAR;
	int i;

	if (nr)
		offset1 = OVCW1B0SAR;

	for (i = 0; i < num;i++) {
		writel((uint32_t)(dma[2 * i].vir_addr), reg_base + offset1 + i * 4);
		writel((uint32_t)(dma[2 * i + 1].vir_addr), reg_base + 0x1300 + i * 4);
	}
}

void osd_set_bppmode(int nr, int mode)
{
	uint32_t val, offset = OVCW0CR;
	
	if (nr)
		offset = OVCW1CR;
	val = readl(reg_base + offset);
	val &= ~(0x1f << OVCWxCR_BPPMODE);
	val |= (mode << OVCWxCR_BPPMODE);
	writel(val, reg_base + offset);
}

void osd_set_colorkey(int nr, struct osd_colorkey *colorkey)
{
	uint32_t val;

	if (nr < 1)
		return;

	val = readl(reg_base + OVCW1CKCR);
	val = ((colorkey->enable << OVCWxCKCR_KEYEN) |
		   (colorkey->enableBlend << OVCWxCKCR_KEYBLEN) |
		   (colorkey->dircon << OVCWxCKCR_DIRCON) |
		   (colorkey->compkey << OVCWxCKCR_COMPKEY));
	writel(val, reg_base + OVCW1CKCR);
	
	writel(colorkey->colval, reg_base + OVCW1CKR);
}

void osd_set_alpha(int nr, struct osd_alpha *alpha)
{
	uint32_t val;
	
	if (nr < 1)
		return;
	
	val = readl(reg_base + OVCW1CR);
	val &= ~(0x3 << OVCWxCR_BLD_PIX);
	val |= ((alpha->path << OVCWxCR_ALPHA_SEL) |
			(alpha->blendmode << OVCWxCR_BLD_PIX));
	writel(val, reg_base + OVCW1CR);
	
	val = ((alpha->alpha0_r << OVCWxPCCR_ALPHA0_R) |
		   (alpha->alpha0_g << OVCWxPCCR_ALPHA0_G) |
		   (alpha->alpha0_b << OVCWxPCCR_ALPHA0_B) |
		   (alpha->alpha1_r << OVCWxPCCR_ALPHA1_R) |
		   (alpha->alpha1_g << OVCWxPCCR_ALPHA1_G) |
		   (alpha->alpha1_b << OVCWxPCCR_ALPHA1_B));
	writel(val, reg_base + OVCW1PCCR);
}

void osd_set_pixel_swap(int nr, struct osd_swap *swap)
{
	uint32_t val, offset = OVCW0CR;
	
	if (nr)
		offset = OVCW1CR;
	
	val = readl(reg_base + offset);
	if (swap->swap == OSD_EXCHANGERB) {
		if (swap->enable)
			val |= (0x1 << OVCWxCR_RBEXG);
		else
			val &= ~(0x1 << OVCWxCR_RBEXG);
	} else {
		if (swap->enable)
			val |= (0x1 << OVCWxCR_HAWSWP(swap->swap));
		else
			val &= ~(0x1 << OVCWxCR_HAWSWP(swap->swap));
	}
	writel(val, reg_base + offset);
}

void osd_set_scaler_mode(enum scaler_mode mode)
{
	uint32_t val;

	val = readl(reg_base + OVCDCR);
	val &= ~(0x1 << OVCDCR_ScalerMode);
	val |= (mode << OVCDCR_ScalerMode);
	writel(val, reg_base + OVCDCR);
}

void osd_set_prescaler_param(struct prescaler_para *para)
{
	uint32_t val;
	int i;

	val = (para->window << 20) | (para->format << 1);
	writel(val, reg_base + OVCPSCCR);
	val = ((para->x - 1)<< 0) | ((para->y - 1) << 16);
	writel(val, reg_base + OVCPSCPCR);
	writel(para->x, reg_base + OVCPSVSSR);
	if (para->format != OSD_IMAGE_YUV_420SP)
		for (i = 0; i < para->dma->dma_num; i++)
			writel((uint32_t)(para->dma->buf[i].vir_addr), reg_base + OVCPSB0SAR + i * 4);
	else {
		for (i = 0; i < para->dma->dma_num / 2; i++) {
			writel((uint32_t)(para->dma->buf[2 * i].vir_addr), reg_base + OVCPSB0SAR + i * 4);
			writel((uint32_t)(para->dma->buf[2 * i + 1].vir_addr), reg_base + OVCPSCB0SAR + i * 4);
		}
	}
}

void osd_prescaler_bppmode(int mode)
{
	int val;

	val = readl(reg_base + OVCPSCCR);
	val &= ~(0x1f << 1);
	val |= ((mode & 0x1f) << 1);
	writel(val, reg_base + OVCPSCCR);
}

void osd_prescalser_cood(int x, int y)
{
	writel((x - 1) | ((y - 1) << 16), reg_base + OVCPSCPCR);
}

void osd_prescaler_enable(int flags)
{
	uint32_t val;

	val = readl(reg_base + OVCPSCCR);
	val &= ~(0x1);	
	val |= flags;
	writel(val, reg_base + OVCPSCCR);
}

void osd_prescaler_dma_mode(int nr, struct osd_buf_mode *mode)
{
	uint32_t val, offset = OVCPSCCR;
	
	val = readl(reg_base + offset);
	val &= ~(0x1f << OVCWxCR_BUFNUM);
	val |= (((mode->bufsel & 0x3) << OVCWxCR_BUFSEL) |
		   ((mode->bufauto & 0x1) << OVCWxCR_BUFAUTOEN) |
		   ((mode->bufnum & 0x3) << OVCWxCR_BUFNUM));
	writel(val, reg_base + offset);
}

#if 0
void osd_set_scaler_input_size(int height, int width)
{
	uint32_t val;

	val = ((height - 1) << SCALER_INPUT_FRAME_VER) |
		  ((width - 1) << SCALER_INPUT_FRAME_HOR);
	writel(val, reg_base + SCLIFSR);
}

void osd_set_scaler_output_size(int height, int width)
{
	uint32_t val;

	val = ((height - 1) << SCALER_OUTPUT_FRAME_VER) |
		  ((width - 1) << SCALER_OUTPUT_FRAME_HOR);
	writel(val, reg_base + SCLOFSR);
}

void osd_set_scaler_output_offset(int height, int width)
{
	uint32_t val;

	val = ((height - 1) << SCALER_OFFSET_FRAME_VER) |
		  ((width - 1) << SCALER_OFFSET_FRAME_HOR);
	writel(val, reg_base + SCLOFOR);
}

void osd_set_height_scaler(struct osd_scaler *scaler)
{
	uint32_t val;

	val = readl(reg_base + SCLCR);
	val &= ~(0xffff << 16);
	val |= ((scaler->roundpolicy << SCALER_CTRL_VROUND) |
			(scaler->enable << SCALER_CTRL_VPASSBY) |
			((scaler->zoomout & 0xf) << SCALER_CTRL_VSCALING_ZOOMOUT) |
			((scaler->zoomin & 0x3ff) << SCALER_CTRL_VSCALING_ZOOMIN));
	writel(val, reg_base + SCLCR);
}

void osd_set_width_scaler(struct osd_scaler *scaler)
{
	uint32_t val;

	val = readl(reg_base + SCLCR);
	val &= ~(0xffff);
	val |= ((scaler->roundpolicy << SCALER_CTRL_HROUND) |
		    (scaler->enable << SCALER_CTRL_HPASSBY) |
			((scaler->zoomout & 0xf) << SCALER_CTRL_HSCALING_ZOOMOUT) |
			((scaler->zoomin & 0x3ff) << SCALER_CTRL_HSCALING_ZOOMIN));
	writel(val, reg_base + SCLCR);
}
#endif

void osd_set_pallete(int nr, struct osd_pallete *pallete)
{
	uint32_t val = 0, offset = OVCW0PAL0;
	int i;

	if (nr)
		offset = OVCW1PAL0;

	if (!pallete || !pallete->table)
		return;

	if (pallete->tableLength) {
		val = readl(reg_base + OVCPCR);
		if (nr) {
			val &= ~(0x7 << OVCPCR_W1PALFM);
			val |= (pallete->format << OVCPCR_W1PALFM);
		} else {
			val &= ~(0x7 << OVCPCR_W0PALFM);
			val |= (pallete->format << OVCPCR_W0PALFM);
		}
		val |= (0x1 << OVCPCR_UPDATE_PAL);
		writel(val, reg_base + OVCPCR);

		for (i = 0; i < pallete->tableLength; i++)
			writel(pallete->table[i], reg_base + offset + 4 * i);
		val = readl(reg_base + OVCPCR);
		val &= ~(0x1 << OVCPCR_UPDATE_PAL);
		writel(val, reg_base + OVCPCR);
	}
}

void osd_set_pre_pallete(struct osd_pallete *pallete)
{
	uint32_t val = 0, i;

	if (!pallete || !pallete->table)
		return;

	if (pallete->tableLength) {
		val = readl(reg_base + OVCPSCCR);
		val &= ~(0x7 << 28);
		val |= (pallete->format << 28);
		writel(val, reg_base + OVCPSCCR);
		for (i = 0; i < pallete->tableLength; i++)
			writel(pallete->table[i],  reg_base + 0x1F80 + 4 * i);
	}
}

void osd_set_global_load(int enable)
{
	uint32_t val;

	val = readl(reg_base + OVCDCR);
	val &= ~(0x1 << OVCDCR_LOAD_EN);
	val |= (enable << OVCDCR_LOAD_EN);
	writel(val, reg_base + OVCDCR);
}

void osd_set_output_format(struct osd_output_format *format)
{
	int val;

	val = readl(reg_base + OVCDCR);
	val &= (~0x3);
	val |= ((format->interlace << OVCDCR_Interlace) |
		   (format->iftype << OVCDCR_IfType));
	writel(val, reg_base + OVCDCR);
}

void osd_win_init(struct osd_win_cfg *cfg)
{
	if ((cfg->nr != 0) && (cfg->nr != 1)) {
		ids_err("Not support this windows: %d\n", cfg->nr);
		return;
	}
	/* set dma mode */
	osd_set_dma_mode(cfg->nr, &cfg->buf_mode);
	/* set bpp mode */
	osd_set_bppmode(cfg->nr, cfg->format);
	/* set xy cood */
	osd_set_coord(cfg->nr, &cfg->coord);
	/* vm_width */
	osd_set_vm_width(cfg->nr, cfg->coord.rightx - cfg->coord.leftx);
	/* set mapcolor */
	osd_colormap_ctrl(cfg->nr, cfg->mapcolor.enable, cfg->mapcolor.color);
	/* set dma buf addr */
	osd_set_dma_channel(cfg->nr, cfg->dma.dma_num, cfg->dma.buf);
	/* set swap */
	osd_set_pixel_swap(cfg->nr, &cfg->swap);
	/* set pallete */
	osd_set_pallete(cfg->nr, &cfg->pallete);
	/* if win1, we need to set alpha and colorkey */
	if (cfg->nr) {
		osd_set_alpha(cfg->nr, &cfg->alpha);
		osd_set_colorkey(cfg->nr, &cfg->colorkey);
	}
}

void osd_open(int nr)
{
	osd_enable_ctrl(nr, 1);
}

void osd_close(int nr)
{
	osd_enable_ctrl(nr, 0);
}

void osd_init(struct osd_cfg *cfg)
{
	uint32_t val;

	val = (cfg->ituInterface << OVCDCR_Interlace) | 
		  (cfg->interlaced << OVCDCR_IfType) |
		  (1 << OVCDCR_LOAD_EN) | (0xf << OVCDCR_WaitTime);
	if (cfg->mode < BOTH_CHANNEL)
		val |= ((cfg->mode + 1) << OVCDCR_SP_LOAD_EN) << 0;
	writel(val, reg_base + OVCDCR);
	/* In normal mode, cannot update palette memory */
	osd_palette_ctrl(DISABLE);
	/* set background color blue */
	osd_set_bg_color(cfg->gbcolor);
	/* set scale xy */
	osd_set_scale_xy(cfg->screenWidth, cfg->screenHeight);
}

void osd_deinit(void)
{

}
