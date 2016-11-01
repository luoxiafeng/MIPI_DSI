#include <ids/ids.h>
#include <asm/io.h>
#include <common.h>
#include <irq.h>
#include "../common/lcd_param.c"

uint32_t reg_base = IDS0_BASE_ADDR;
uint32_t irq_num = 0;
char *ids_eitf;
static int pix_clk = 0;
struct lcd_param *bak_param = NULL;

static int rgb_value[6] = {
	0x6, 0x9, 0x12, 0x18, 0x21, 0x24 
};

static struct reg_check reg[] = {
	REG_CHECK_INIT(LCDCON1, 0x0003FF01),
	REG_CHECK_INIT(LCDCON2, 0x07FF07FF),
	REG_CHECK_INIT(LCDCON3, 0x07FF07FF),
	REG_CHECK_INIT(LCDCON4, 0x07FF07FF),
	REG_CHECK_INIT(LCDCON5, 0xBF001FC0),
	REG_CHECK_INIT(LCDCON6, 0x07FF07FF),
};

int lcd_reg_check(void)
{
	int i;

	for (i = 0;i < ARRAY_SIZE(reg);i++)
		reg[i].addr += reg_base;
	
   if (common_reg_check(reg, ARRAY_SIZE(reg), 0xffffffff) || common_reg_check(reg, ARRAY_SIZE(reg), 0))
		return -1;

	return 0;
}

void lcd_set_output_if(enum EXT_DISPLAY_TYPE flags)
{
	uint32_t val;

	val = readl(reg_base + LCDCON5);
	val &= ~(0x3 << LCDCON5_DSPTYPE);
	val |= (flags << LCDCON5_DSPTYPE);
	writel(val, reg_base + LCDCON5);
}

void lcd_set_rgb_cfgorder(int order)
{
	uint32_t val;

	val = readl(reg_base + LCDCON5);
	val &= ~(0x3f << LCDCON5_RGBORDER);
	val |= (order << LCDCON5_RGBORDER);
	writel(val, reg_base + LCDCON5);
}

void lcd_set_cfgorder(int order)
{
	uint32_t val;

	val = readl(reg_base + LCDCON5);
	val &= ~(0x07 << LCDCON5_CONFIGORDER);
	val |= (order << LCDCON5_CONFIGORDER);
	writel(val, reg_base + LCDCON5);
}

void lcd_in_polarity(void)
{
	uint32_t val, value;

	val = readl(reg_base + LCDCON5);
	value = val & (0x1f << LCDCON5_INVVDEN);
	value = ~value;
	val &= ~(0x1f << LCDCON5_INVVDEN);
	val |= value;
	writel(val, reg_base + LCDCON5);
}


static void lcd_disable_auto_change(void)
{
	writel((0xf << LCDVCLKFSR_CDOWN) | (0x1 << LCDVCLKFSR_RFRM_NUM),
			reg_base + LCDVCLKFSR);
}

void lcd_set_panel(int lineval, int hozval)
{
	uint32_t lcdcon = 0;
	lcdcon = ((lineval - 1) << LCDCON6_LINEVAL) | 
			 ((hozval - 1) << LCDCON6_HOZVAL);
	writel(lcdcon, reg_base + LCDCON6);
}

void lcd_set_vsync_polarity(int flags)
{
	uint32_t val;
	
	val = readl(reg_base + LCDCON5);
	val &= ~(0x1 << LCDCON5_INVVFRAME);
	val |= (flags << LCDCON5_INVVFRAME);
	writel(val, reg_base + LCDCON5);
}

static void lcd_set_reg(struct lcd_param *lcd)
{
	uint32_t lcdcon = 0;
	int div;
	
	lcdcon = (0<<LCDCON1_VMMODE) | (3<<LCDCON1_PNRMODE) |
			 (12<<LCDCON1_STNBPP) | (0<<LCDCON1_ENVID);
	div = pix_clk / (lcd->mPixelClock * 1000);
	if (div < 1)
		div = 1;

	lcdcon |= (((div - 1) & 0x3ff) << LCDCON1_CLKVAL);
	writel(lcdcon, reg_base + LCDCON1);

	lcdcon = ((lcd->mVBackPorch - 1)<< LCDCON2_VBPD) | 
			 ((lcd->mVFrontPorch - 1)<< LCDCON2_VFPD);
	writel(lcdcon, reg_base + LCDCON2);

	lcdcon = ((lcd->mVSyncPulseWidth - 1)<< LCDCON3_VSPW) |
			 ((lcd->mHSyncPulseWidth - 1)<< LCDCON3_HSPW);
	writel(lcdcon, reg_base + LCDCON3);

	lcdcon = ((lcd->mHBackPorch - 1)<< LCDCON4_HBPD) |
			 ((lcd->mHFrontPorch - 1)<< LCDCON4_HFPD);
	writel(lcdcon, reg_base + LCDCON4);

	lcdcon = (lcd->rgb_bpp << LCDCON5_RGB565) |
			 (rgb_value[lcd->rgb_seq] << LCDCON5_RGBORDER) |
			 (TFT_LCD << LCDCON5_DSPTYPE) |
			 (lcd->VclkPolarity << LCDCON5_INVVCLK) |
			 (lcd->mHSyncPolarity << LCDCON5_INVVLINE) |
			 (lcd->mVSyncPolarity << LCDCON5_INVVFRAME) |
			 (lcd->VDPolarity << LCDCON5_INVVD) |
			 (lcd->VdenPolarity << LCDCON5_INVVDEN) |
			 (lcd->Vpwren << LCDCON5_INVPWREN) |
			 (lcd->Pwren << LCDCON5_PWREN);
	writel(lcdcon, reg_base + LCDCON5);

	lcdcon = ((lcd->mVActive - 1) << LCDCON6_LINEVAL) | 
			 ((lcd->mHActive - 1) << LCDCON6_HOZVAL);
	writel(lcdcon, reg_base + LCDCON6);

	/* unmask all interrupt */
	writel(0x0, reg_base + IDSINTMSK); 
	/* VCLK frequency auto change */
	lcd_disable_auto_change();
}

int lcd_frame_irq(void)
{
	uint32_t status;
	uint32_t vclkfsr;
	int ret = -1;

	status = readl(reg_base + IDSINTPND);

	if (status & (1 << IDSINTPND_LCDINT)) {
		ids_dbg("success to send a frame\n");
		ret = 0;
	}

	if (status & (1 << IDSINTPND_VCLKINT))
		ids_dbg("too fast to send frames\n");

	if (status & (1 << IDSINTPND_OSDERR))
		ids_err("OSD occur error, need to reset OSD\n");
	
	vclkfsr = readl(reg_base + LCDVCLKFSR);
	if (vclkfsr & (1 << LCDVCLKFSR_VCLKFAC)) {
		ids_dbg("CDOWN = 0x%x, RFRM_NUM = 0x%x, CLKVAL = 0x%x\n",
			   (vclkfsr >> LCDVCLKFSR_CDOWN) & 0xF,
			   (vclkfsr >> LCDVCLKFSR_RFRM_NUM) & 0x3F,
			   (vclkfsr >> LCDVCLKFSR_CLKVAL) & 0x3FF);
		writel(vclkfsr, reg_base + LCDVCLKFSR);
	}
	writel(status, reg_base + IDSSRCPND);
	writel(status, reg_base + IDSINTPND);

	irq_num++;
	return ret;
}

void lcd_wait_frame(void)
{
	while (lcd_frame_irq())
		;
}

int lcd_wait_frame_timeout(int timeout)
{
	uint64_t tt;
	int flag=0;
	tt =get_ticks();	
	while (lcd_frame_irq()){
		if((get_ticks()-tt)>timeout){
			flag=1;
			printf("timeout\n");
			break;	
		}
	}
	if(flag)
	   return 1;
	else
	  return 0;	
}

void lcd_set_serial_cfg(struct lcd_serial *serial)
{
        int val, div, col;
        if (!serial)
                return;

        val = readl(reg_base + LCDCON5);
        val &= ~(0x7 << 17);
        val |= (((serial->type & 0x3) << 18) | ((serial->enable & 0x1) << 17));
        writel(val, reg_base + LCDCON5);
        ids_err("lcd serial cfg is 0x%x\n", val);
        if (serial->enable) {
                if (serial->type == LCD_SERIAL_RGB)
                        col = 3;
                else
                        col = 4;
        
                if (!bak_param)
                        return;
                div = pix_clk / (bak_param->mPixelClock * 1000 * col);
                if (div < 1)
                        div = 1;

                val = readl(reg_base + LCDCON1);
                val &= ~(0x3ff << LCDCON1_CLKVAL);
                val |= ((div - 1) << LCDCON1_CLKVAL);
                ids_err("serial clock is %d, val is 0x%x\n", div, val);
                writel(val, reg_base + LCDCON1);

                lcd_set_screen_size(col * bak_param->mHActive, bak_param->mVActive);
        }
}

void lcd_get_screen_size(int *width, int *height)
{
	uint32_t val;

	val = readl(reg_base + LCDCON6);
	*width = (val & 0xfff) + 1;
	*height = ((val >> LCDCON6_LINEVAL) & 0xfff) + 1;
}

void lcd_set_screen_size(int width, int height)
{
	uint32_t val = 0;

	val = (((height - 1) & 0x7ff) << LCDCON6_LINEVAL) | ((width - 1) & 0x7ff);
	writel(val, reg_base + LCDCON6);
}

int lcd_close(void)
{
	lcd_enable(0);
	return 0;
}

int lcd_open(char *name)
{
	int i;

	for (i = 0;i < ARRAY_SIZE(panel);i++)
		if (!strcmp(name, panel[i].name)) 
			break;

	if (i == ARRAY_SIZE(panel))
		return -1;
	
	bak_param = &panel[i];
	lcd_set_reg(&panel[i]);
	return 0;
}

void lcd_irq_mode(void)
{
	int ret;

	ret = request_irq(IDS0_INT_ID, (irq_handler_t)lcd_frame_irq, "lcd");
	if (ret < 0) {
		ids_err("Lcd request irq err\n");
		return;
	}
		
}

void lcd_enable(int enable)
{
	uint32_t val;

	val = readl(reg_base + LCDCON1);
	if (enable)
		val |= (1 << LCDCON1_ENVID);
	else
		val &= ~(1 << LCDCON1_ENVID);
	writel(val, reg_base + LCDCON1);
}

int lcd_set_pixel_clk(int rate)
{
#ifdef CONFIG_COMPILE_RTL
    uint32_t ref_clock;
    uint32_t ratio; 
    ref_clock = 297000000;
    ratio = ref_clock / rate - 1;
    /*set ids-eitf 60MHZ*/
    module_set_clock(ids_eitf, "epll", ref_clock, 0);
    pix_clk = ref_clock / (ratio + 1);
#endif    	
#if 0
    
    uint32_t ratio; 
    //ref_clock = 60000000;
    ref_clock = 300000000;
	ratio = ref_clock / rate - 1;
    /*set ids-eitf 60MHZ*/
	module_set_clock(ids_eitf, "epll", ref_clock, 0);
	pix_clk = ref_clock / (ratio + 1);
#endif
	printf("pix_clk %d\n",pix_clk);
	return pix_clk;
}

void lcd_exchang_ids(int ids_num)
{
	ids_err("Q3 only support ids1\n");
}

int lcd_init(int ids_num)
{
	if (ids_num) { 
		reg_base = IDS1_BASE_ADDR;
		module_enable("ids1");
		module_reset("ids0");
		ids_eitf = "ids1-eitf";
#ifdef CONFIG_COMPILE_FPGA
		pix_clk = 40000000;
#else
		//pix_clk = 297000000;
		pix_clk = 56000000;
#endif
		//module_set_clock("ids1", "dpll", 237600000, 0);
		module_set_clock("ids1", "dpll", 108000000, 0);
	} else {
		module_enable("ids0");
		module_reset("ids0");
		reg_base = IDS0_BASE_ADDR;
		ids_eitf = "ids0-eitf";
#ifdef CONFIG_COMPILE_FPGA
		pix_clk = 40000000;
#else
		//pix_clk = 297000000;
		pix_clk = 56000000;
#endif
		//module_set_clock("ids0", "dpll", 237600000, 0);
		module_set_clock("ids0", "dpll", 108000000, 0);
	}

	module_set_clock(ids_eitf, "epll", pix_clk, 0);
	/* set power 28 pullup, and
	 * set backlight 27 pullup
	 * set pwm2 pullup */
	
	gpio_switch_func_by_module("rgb0",GPIO_FUNC_MODE_DEFAULT,CPU_TYPE_AP);
   	//pads_set_mode(PADSRANGE(0, 27), PADS_MODE_CTRL, 0);
	writel(0x0, reg_base + IDSINTMSK);
	lcd_disable_auto_change();
	return 0;
}
