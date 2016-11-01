#include <ids/ids_api.h>
#include <asm/io.h> 
#include <common.h> 

static int type = IDS_RGB_BPP28_4A888;

int ids_init(void)
{
	if (lcd_init(0) < 0)
		return -1;

	if (lcd_open("KR070LB0S_1024_600") < 0)
		return -1;

	osd_common_init(WINDOWS0, LCD_MODE, NULL);
	osd_set_scaler_mode(SCALER_BEFORE_OSD_MODULE);
	return 0;
}

static int32_t YCbCr2RGB[] = {16, 0, 298, 0, 409, 298, 1984, 1840, 298, 516, 0};
int ids_set_format(enum format_type format)
{
	struct osd_yuv yuv;

	if (format < IDS_RGB_BPP8_1A232) {
		printf("Now, can't support this format: %d\n", format);
		return -1;
	}

	type = format;
	osd_set_bppmode(WINDOWS0, format);
	if (format == IDS_YUV_420SP) {
		yuv.enable = 1;
		memcpy(&(yuv.out_oft_b), YCbCr2RGB, 11 * 4);
		osd_set_yuvtorgb(&yuv);
	}
	return 0;
}

void ids_set_vm_width(int width)
{
	osd_set_vm_width(WINDOWS0, width);
}

void ids_set_resolution(int hozval, int lineval)
{
	int x, y;
	
	lcd_get_screen_size(&x, &y);
	osd_set_scale_xy(x, y);
	if (hozval > x)
		hozval = x;
	if (lineval > y)
		lineval = y;

	osd_set_end_xy(WINDOWS0, hozval, lineval);
}

int ids_set_dma_addr(uint8_t *yaddr, uint8_t *uvaddr)
{
	struct dma_buffer dma;
	
	if (!yaddr)
		return -1;

	if ((type == IDS_YUV_420SP) && !uvaddr)
		return -1;

	dma.vir_addr = yaddr;
	osd_set_dma_channel(WINDOWS0, 1, &dma);
	if (type == IDS_YUV_420SP)
		//writel((uint32_t)uvaddr, 0x23001300);
		writel((uint32_t)uvaddr, 0x22301300);
	return 0;
}

void ids_enable(int enable)
{
	if (enable)
		osd_open(WINDOWS0);
	else
		osd_close(WINDOWS0);

	lcd_enable(enable);
}
