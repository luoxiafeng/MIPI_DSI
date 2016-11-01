#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <ids/ids.h>
#include <cvbs.h>

static void ids_convert_RGBTOYUV(struct ids_mannual_image *imagecfg)
{
	int i, j; 
	uint8_t *bufY, *bufUV, *bufRGB;
	uint8_t y, u, v, r, g, b;

	bufY = imagecfg->YUV;
	bufUV = bufY + imagecfg->width * imagecfg->height;

	for (j = 0; j < imagecfg->width; j++) {
		bufRGB = (uint8_t *)(imagecfg->mem + imagecfg->width
				 * (imagecfg->height - 1 - j) * 3);
		for (i = 0; i < imagecfg->width; i++) {
			r = *(bufRGB++);
			g = *(bufRGB++);
			b = *(bufRGB++);

			y = (uint8_t)((66 * r + 129 * g +  25 * b + 128) >> 8) + 16;
			u = (uint8_t)((-38 * r -  74 * g + 112 * b + 128) >> 8) + 128;          
			v = (uint8_t)((112 * r -  94 * g -  18 * b + 128) >> 8) + 128;
			*(bufY++) = max(0, min(y, 255));
			if (i % 2) {
				*(bufUV++) = u;
				*(bufUV++) = v;
			}
		}
	}
	ids_err("ids_convert_RGBTOYUV OK\n");
}

int ids_draw_image(struct ids_mannual_image *imagecfg, int flags)
{
	if (!imagecfg->set) {
		imagecfg->format = OSD_IMAGE_RGB_BPP24_888;
		imagecfg->type = OSD_TEST_IMAGE_TYPE_HOR_BAR;
		if (flags == CVBS_MODE)
			tvif_get_display(&imagecfg->width, &imagecfg->height);
		else if (flags == LCD_MODE)
			lcd_get_screen_size(&imagecfg->width, &imagecfg->height);
		imagecfg->mVBar = 8;
	}
	/* if output format = OSD_IMAGE_YUV_420SP, we need first draw a RGB
	 * image, then convert YUV420sp */
	if (imagecfg->format == OSD_IMAGE_YUV_420SP) {
		imagecfg->YUV = (uint8_t *)imagecfg->mem;
		imagecfg->mem = NULL;
	}

	if (!imagecfg->mem)
		imagecfg->mem = (uint32_t *)malloc(imagecfg->width * imagecfg->height * 4);
	if (!imagecfg->mem) {
		ids_err("malloc mem err\n");
		return -1;
	}
	ids_fill_framebuffer(imagecfg);
	if (imagecfg->format == OSD_IMAGE_YUV_420SP)
		ids_convert_RGBTOYUV(imagecfg);
	return 0;
}

void osd_dma_init(struct osd_dma *dma, int flags, int dma_num)
{
	int width, height;
	int i;

	if (dma_num > 8) {
		ids_err("Framebuffer in [1, 4]\n");
		return;
	}

	if (flags == CVBS_MODE)
		tvif_get_display(&width, &height);
	else if (flags == LCD_MODE)
		lcd_get_screen_size(&width, &height);
	dma->dma_num = dma_num;
	for (i = 0; i < dma_num; i++) {
		dma->buf[i].size = width * height * 4;
		dma->buf[i].vir_addr = malloc(dma->buf[i].size);        
		if (!dma->buf[i].vir_addr) {
			ids_err("osd_dma_init: malloc err\n");
			return;
		}
		dma->buf[i].phy_addr = (uint32_t)dma->buf[i].vir_addr;
	}
}

void osd_common_init(int nr, enum osd_mode mode, struct osd_dma *dma)
{
	struct osd_win_cfg win;
	struct osd_cfg cfg;
	
	memset(&win,0,sizeof(struct osd_win_cfg));
	/* set osd global reg */
	cfg.mode = BOTH_CHANNEL;
	cfg.interlaced = PROGRESSIVE;
	cfg.ituInterface = IF_RGB;
	if (mode == LCD_MODE)
		lcd_get_screen_size(&cfg.screenWidth, &cfg.screenHeight);
	else if (mode == CVBS_MODE) {
		//tvif_get_display(&cfg.screenWidth, &cfg.screenHeight);
#if 0
		cfg.screenWidth = 1;
		cfg.screenHeight = 1;
#else
		tvif_get_display(&cfg.screenWidth, &cfg.screenHeight);
#endif
	}
	//cfg.gbcolor = 0xff << 8;
	cfg.gbcolor = 0;
	osd_init(&cfg);
	/* init osd win */
	win.nr = nr;
	win.buf_mode.bufsel = FETCH_BUF0;
	win.buf_mode.bufauto = BUF_FIXED;
	win.buf_mode.bufnum = BUF_NUM1;
	win.pallete.tableLength = 0;
	win.exchangeRB = DISABLE;
	win.swap.enable = DISABLE;
	win.format = OSD_IMAGE_RGB_BPP24_888;
	win.coord.leftx = 0;
	win.coord.lefty = 0;
	if (mode == LCD_MODE)
		lcd_get_screen_size(&win.coord.rightx, &win.coord.righty);
	else if (mode == CVBS_MODE)
		tvif_get_display(&win.coord.rightx, &win.coord.righty);
	win.mapcolor.enable = DISABLE;
	win.mapcolor.color = 0;
	win.swap.enable = DISABLE;
	win.swap.swap = 0;
	if (dma)
		memcpy(&win.dma, dma, sizeof(struct osd_dma));
	else {
		win.dma.dma_num = 0;
		ids_err("Win%d dma addr set NULL\n", nr);
	}

	/* only window1 need to set alpha and colorkey */
	if (nr) {
		win.alpha.path = ALPHA_KEY;
		win.alpha.blendmode = ALPHA_PLANE_BLENDING;
		win.alpha.alpha0_r = 0xf;
		win.alpha.alpha0_g = 0xf;
		win.alpha.alpha0_b = 0xf;
		win.alpha.alpha1_r = 0x0;
		win.alpha.alpha1_g = 0x0;
		win.alpha.alpha1_b = 0x0;

		win.colorkey.enable = DISABLE;
		win.colorkey.colval = DISABLE;
	}
	osd_win_init(&win);
}

static struct bpp_color_params bpps[] = {
	BPP_COLOR(OSD_IMAGE_PAL_BPP1, 1, BPPSET(PALLETE_BPP, 0, 0, 1)),
	BPP_COLOR(OSD_IMAGE_PAL_BPP2, 2, BPPSET(PALLETE_BPP, 0, 0, 2)),
	BPP_COLOR(OSD_IMAGE_PAL_BPP4, 4, BPPSET(PALLETE_BPP, 0, 0, 4)),	
	BPP_COLOR(OSD_IMAGE_PAL_BPP8, 8, BPPSET(PALLETE_BPP, 0, 0, 8)),
	BPP_COLOR(OSD_IMAGE_RGB_BPP8_1A232, 8, BPPSET(ARGB_BPP, 2, 3, 2)),
	BPP_COLOR(OSD_IMAGE_RGB_BPP16_565, 16, BPPSET(RGB_BPP, 5, 6, 5)),
	BPP_COLOR(OSD_IMAGE_RGB_BPP16_1A555, 16, BPPSET(ARGB_BPP, 5, 5, 5)),
	BPP_COLOR(OSD_IMAGE_RGB_BPP16_I555, 16, BPPSET(IRGB_BPP, 5, 5, 5)),
	BPP_COLOR(OSD_IMAGE_RGB_BPP18_666, 32, BPPSET(RGB_BPP, 6, 6, 6)),
	BPP_COLOR(OSD_IMAGE_RGB_BPP18_1A665, 32, BPPSET(ARGB_BPP, 6, 6, 5)),
	BPP_COLOR(OSD_IMAGE_RGB_BPP19_1A666, 32, BPPSET(ARGB_BPP, 6, 6, 6)),
	BPP_COLOR(OSD_IMAGE_RGB_BPP24_888, 32, BPPSET(RGB_BPP, 8, 8, 8)),
	BPP_COLOR(OSD_IMAGE_RGB_BPP24_1A887, 32, BPPSET(ARGB_BPP, 8, 8, 7)),
	BPP_COLOR(OSD_IMAGE_RGB_BPP25_1A888, 32, BPPSET(ARGB_BPP, 8, 8, 8)),
	BPP_COLOR(OSD_IMAGE_RGB_BPP28_4A888, 32, BPPSET(A4RGB_BPP, 8, 8, 8)),
	BPP_COLOR(OSD_IMAGE_RGB_BPP16_4A444, 16, BPPSET(A4RGB_BPP, 4, 4, 4)),
	BPP_COLOR(OSD_IMAGE_RGB_BPP32_8A888, 32, BPPSET(A8RGB_BPP, 8, 8, 8)),
	BPP_COLOR(OSD_IMAGE_YUV_420SP, 0, 0),
	BPP_COLOR(OSD_IMAGE_RGB_BPP32_888A, 32, BPPSET(RGBA_BPP, 8, 8, 8)),
	BPP_COLOR(OSD_IMAGE_RGB_BPP16_555A, 16, BPPSET(RGBA_BPP, 5, 5, 5)),
	BPP_COLOR(OSD_IMAGE_RGB_BPP16_555I, 16, BPPSET(RGBI_BPP, 5, 5, 5)),
};

static uint32_t ids_get32_color(uint32_t color, int bit)
{
	uint32_t fillcolor = color;

	if (bit <= 8)
		fillcolor = (fillcolor << 24) | (fillcolor << 16) |
					(fillcolor << 8) | fillcolor;
	else if (bit == 16)
		fillcolor = (fillcolor << 16) | fillcolor;

	return fillcolor;
}

static uint32_t ids_get_pixcolor(struct bpp_color_params *params,
						int color, int alen)
{
	uint32_t pixelmode = params->pixelmode;
	uint32_t mode = BPPGETMODE(pixelmode);
	uint32_t fillcolor;

	switch (mode) {
		case PALLETE_BPP:
			if (params->bitnum == 1 && color)
				fillcolor = 0xff;
			else
				fillcolor = BPP888SETCOLOR(color, pixelmode);
			if (params->bitnum == 4)
				fillcolor = (fillcolor << 4) | fillcolor;
			else if (params->bitnum == 2)
				fillcolor = (fillcolor << 6) | (fillcolor << 4) |
							(fillcolor << 2) | fillcolor;
			break;
		case RGB_BPP:
			fillcolor = BPP888SETCOLOR(color, pixelmode);
			break;
		case ARGB_BPP:
			fillcolor = BPP888SETCOLOR(color, pixelmode);
			fillcolor |= ((alen & 0x1) << BPPRGB(pixelmode));
			break;
		case RGBA_BPP:
			fillcolor = BPP888SETCOLOR(color, pixelmode);
			fillcolor = (fillcolor << 1) | (alen & 0x1);
			break;
		case A4RGB_BPP:
			fillcolor = BPP888SETCOLOR(color, pixelmode);
			fillcolor |= ((alen & 0xf) << BPPRGB(pixelmode));
			break;
		case A8RGB_BPP:
			fillcolor = BPP888SETCOLOR(color, pixelmode);
			fillcolor |= ((alen & 0xff) << BPPRGB(pixelmode));
			break;
		case IRGB_BPP:
			fillcolor = BPP888SETCOLOR(color, pixelmode);
			break;
		case RGBI_BPP:
			fillcolor = BPP888SETCOLOR(color, pixelmode);
			fillcolor <<= 1;
			break;
		default:
			ids_err("pixel mode err\n");
			return -1;
	}
	fillcolor = ids_get32_color(fillcolor, params->bitnum);
	
	return fillcolor;	
}

static int ids_image_fill_color(struct ids_mannual_image *cfg)
{
	struct bpp_color_params *params = &bpps[cfg->format];
	uint32_t bufsize, fillcolor;
	uint32_t *buf32 = (uint32_t *)cfg->mem;
	int i;
	
	if (!cfg->mem)
		return -1;

	bufsize = cfg->width * cfg->height;
	bufsize = bufsize * params->bitnum / 32; // 32bit
	fillcolor = ids_get_pixcolor(params, cfg->color, cfg->alpha);
	if (fillcolor < 0)
		return -1;

	ids_dbg("fillcolor is 0x%x, bufsize is 0x%x\n\n", fillcolor, bufsize);
	for (i = 0; i < bufsize; i++)
		*buf32++ = fillcolor;
	return 0;
}

static int ids_image_hor_color(struct ids_mannual_image *cfg)
{
	struct bpp_color_params *params = &bpps[cfg->format];
	uint32_t nheight, bufsize, fillcolor;
	uint32_t *buf32 = (uint32_t *)cfg->mem;
	int i, j;

	if (!cfg->mem || cfg->mVBar <= 0)
		return -1;
	
	nheight = cfg->height / cfg->mVBar;
	bufsize = cfg->width * cfg->mVBar * params->bitnum / 32;

	for (i = 0;i < nheight;i++) 
		for (j = 0;j < bufsize;j++) {
			fillcolor = ids_get_pixcolor(params,
							0xff << (8 * (i % 3)), cfg->alpha);
			if (fillcolor < 0)
				return -1;
				
			*buf32++ = fillcolor;
		}

	return 0;
}

static int ids_image_ver_color(struct ids_mannual_image *cfg)
{
	struct bpp_color_params *params = &bpps[cfg->format];
	uint32_t bufsize, fillcolor, line;
	uint32_t *buf32 = (uint32_t *)cfg->mem;
	int i, j;

	if (!cfg->mem || cfg->mHBar <= 0)
		return -1;

	bufsize = cfg->width * cfg->height * params->bitnum / 32;
	
	for (i = 0; i < bufsize; i++) {
		j = i % (cfg->width * params->bitnum / 32);
		line = j / (cfg->mHBar * params->bitnum / 32);
		fillcolor = ids_get_pixcolor(params,
						0xff << (8 * (line % 3)), cfg->alpha);
		if (fillcolor < 0)
			return -1;

		*buf32++ = fillcolor;
	}
	return 0;
}

#if defined(CONFIG_COMPILE_FPGA) || defined(CONFIG_COMPILE_RTL)
static const char *imagetype[] = {
   "fill_color",
   "hor_bar",
   "ver_bar",
};
#endif

int ids_fill_framebuffer(struct ids_mannual_image *cfg)
{
#if defined(CONFIG_COMPILE_FPGA) || defined(CONFIG_COMPILE_RTL)
    char name[32];
    int len, val;
#endif   
	if (cfg->format < OSD_IMAGE_PAL_BPP1 || 
			cfg->format > OSD_IMAGE_RGB_BPP16_555I) {
		ids_err("Not support this bpp mode: %d\n", cfg->format);
		return -1;
	}
#if defined(CONFIG_COMPILE_FPGA) || defined(CONFIG_COMPILE_RTL)
   len = cfg->width * cfg->height * 4;
   if (cfg->type == OSD_TEST_IMAGE_TYPE_HOR_BAR)
       val = cfg->mVBar;
   else if (cfg->type == OSD_TEST_IMAGE_TYPE_VER_BAR)
       val = cfg->mHBar;
   else
       val = cfg->color;
   sprintf(name, "ids/osd/%s_%d", imagetype[cfg->type], val);
   if (cfg->alpha)
	   sprintf(name, "ids/osd/%s_%d_%d", imagetype[cfg->type], val, cfg->alpha); 
   if (cfg->format != OSD_IMAGE_RGB_BPP24_888)
	   sprintf(name, "ids/osd/%s_%d_%d", imagetype[cfg->type], val, cfg->format);
   if (cfg->name[0] != '\0') {
	   cfg->fd = open(cfg->name, O_RDONLY);
	   printf("cfg->name is %s\n", cfg->name);
   } else {
	   cfg->fd = open(name, O_RDONLY);
	   printf("name is %s\n", name);
   }
   if (cfg->fd < 0) {
	   ids_err("open file err, %s\n", name);
	   return -1;
   }
   if (read(cfg->fd, cfg->mem, len) < 0) {
	   ids_err("read file err: %s\n", name);
	   return -1;
   }
   close(cfg->fd);
   return 0;
#endif   
	switch (cfg->type) {
		case OSD_TEST_IMAGE_TYPE_FILL_COLOR:
			return ids_image_fill_color(cfg);
		case OSD_TEST_IMAGE_TYPE_HOR_BAR:
			return ids_image_hor_color(cfg);
		case OSD_TEST_IMAGE_TYPE_VER_BAR:
			return ids_image_ver_color(cfg);
		case OSD_TEST_IMAGE_TYPE_GRID:
		default:
			ids_err("Not support this image style: %d\n", cfg->type);
			return 1;
	}
}
