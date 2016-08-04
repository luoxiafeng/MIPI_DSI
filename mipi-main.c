/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Memory Functions
 *
 * Copied from FADS ROM, Dan Malek (dmalek@jlc.net)
 */

#include <common.h>
#include <command.h>
#include <getopt.h>
#include <testbench.h>
#include <malloc.h>
#include <asm/io.h>
#include <ids/ids.h>
#include <ids/mipi.h>
#include <i80.h>

static const char *result[] = {
	"PASS", "FAILED"
};

static const char *casename[] = {
	"mipi dsi register",
	"mipi dsi switch off/on",
	"mipi dsi cmd&video mode switch",
	"mipi dsi in burst 24bpp mode",
	"mipi dsi in burst 18bpp mode",
	"mipi dsi in burst 16bpp mode",
	"mipi dsi in no-burst mode",
	"mipi dsi in cmd 18bpp mode",
	"mipi dsi in lp mode",
	"mipi dsi in HS mode",
	"mipi dsi irq test",
	"mipi dsi in burst 24bpp mode hd",
	"mipi dsi dbi",
	NULL,
};

static void mipitest_show_usage(void)
{
	int i = 0;

	printf("Usage: mipi [OPTION]...\n"
		"\n"
		"-h, --help\t\t\tcommand help\n"
		"-c, --case\t\t\tcase num\n");
	printf("------------------------------------------------\n");
	printf("Supported case:\n");
	printf("\tcase num\t\tcase name\n");
	while (casename[i] != NULL) {
		printf("\t%8d\t\t%s\n", i, casename[i]);
		i++;
	}
}

/* display ver line image */
/*flag
   0:ids not output
   1:ids output
*/
static void mipi_dsi_test_24burst(int flag)
{
        struct ids_mannual_image imagecfg;
        struct osd_dma dma;

        if (lcd_init(0) < 0 || mipi_init(0) < 0)
                return;

        if (lcd_open("KR070LB0S_1024_600") < 0)
                return;

        osd_dma_init(&dma, LCD_MODE, 1);
        memset(&imagecfg, 0, sizeof(struct ids_mannual_image));
        imagecfg.mem = dma.buf[0].vir_addr;

        /* mannual draw image */
        ids_draw_image(&imagecfg, LCD_MODE);
        /* init osd module */
        osd_common_init(WINDOWS0, LCD_MODE, &dma);
        osd_set_scaler_mode(SCALER_BEFORE_OSD_MODULE);
        /* enable osd*/
        osd_open(WINDOWS0);

    mipi_open("KR070LB0S_1024_600");
        //lcd_enable(1);
        lcd_enable(flag);
}

/* display ver line image */
static void mipi_dsi_test_24burst_hd(void)
{
	struct osd_dma dma;
	int fd;
	
	if (lcd_init(0) < 0 || mipi_init(0) < 0)
		return;
	
	if (lcd_open("HD_1920_1080") < 0)
		return;
	
	osd_dma_init(&dma, LCD_MODE, 1);

	fd = open("ids/osd/1920_1080.bmp",O_RDONLY);
	if (fd < 0) {
		ids_err("open file err: %d\n", fd);
		return;
	}
	if (read(fd, dma.buf[0].vir_addr, dma.buf[0].size) !=
			dma.buf[0].size) {
		ids_err("Lcd read file err\n");
		return;
	}
	close(fd);

	/* init osd module */
	osd_common_init(WINDOWS0, LCD_MODE, &dma);
   	osd_set_scaler_mode(SCALER_BEFORE_OSD_MODULE);      
	/* enable osd*/    
	osd_open(WINDOWS0);
	
    	mipi_open("HD_1920_1080");
	lcd_enable(1);
}

static void mipi_dsi_test_18burst(int config_num)
{
	struct ids_mannual_image imagecfg;
	struct osd_dma dma;
	uint8_t buf[2];
	//int i;

	if (lcd_init(0) < 0 || mipi_init(0) < 0)
		return;
	
	if (lcd_open("KR070LB0S_1024_600") < 0)
		return;
	
	osd_dma_init(&dma, LCD_MODE, 1);
	memset(&imagecfg, 0, sizeof(struct ids_mannual_image));
	imagecfg.set = 1;
	imagecfg.mem = dma.buf[0].vir_addr;
	imagecfg.format = OSD_IMAGE_RGB_BPP18_666;              
	imagecfg.type = OSD_TEST_IMAGE_TYPE_HOR_BAR;            
	lcd_get_screen_size(&imagecfg.width, &imagecfg.height);
	imagecfg.mVBar = 8;                                     

	/* mannual draw image */
	ids_draw_image(&imagecfg, LCD_MODE);
	/* init osd module */
	osd_common_init(WINDOWS0, LCD_MODE, &dma);
        osd_set_scaler_mode(SCALER_BEFORE_OSD_MODULE);
	/* enable osd*/    
	osd_open(WINDOWS0);
	/* enable lcd */	
	//lcd_enable(1);
	
	if (mipi_open("KR070LB0S_1024_600") < 0) 
		return;
	/* set cmd for change pixel format to RGB666 */
	buf[0] = 0x3a;
	buf[1] = 0x66;
	mipi_send_ctrl(buf, 2);
	osd_set_bppmode(0, OSD_IMAGE_RGB_BPP18_666);	

/*
	for (i = COLOR_CODE_18BIT_CONFIG1; i < COLOR_CODE_24BIT; i++) {
		lcd_set_cfgorder(i + 1);
		mipi_set_pixel_format(i);
#ifndef CONFIG_COMPILE_RTL
		udelay(2000000);
#endif
	}
*/
	lcd_set_cfgorder(config_num + 1);
	mipi_set_pixel_format(config_num);
	lcd_enable(1);
}

static void mipi_dsi_test_16burst(int config_num)
{
	struct ids_mannual_image imagecfg;
	struct osd_dma dma;
	uint8_t buf[2];
	//int i;

	if (lcd_init(0) < 0 || mipi_init(0) < 0)
		return;
	
	if (lcd_open("KR070LB0S_1024_600") < 0)
		return;
	
	osd_dma_init(&dma, LCD_MODE, 1);
	memset(&imagecfg, 0, sizeof(struct ids_mannual_image));
	imagecfg.set = 1;
	imagecfg.mem = dma.buf[0].vir_addr;
	imagecfg.format = OSD_IMAGE_RGB_BPP16_565;              
	imagecfg.type = OSD_TEST_IMAGE_TYPE_HOR_BAR;            
	lcd_get_screen_size(&imagecfg.width, &imagecfg.height);
	imagecfg.mVBar = 8;                                     

	/* mannual draw image */
	ids_draw_image(&imagecfg, LCD_MODE);
	/* init osd module */
	osd_common_init(WINDOWS0, LCD_MODE, &dma);
        osd_set_scaler_mode(SCALER_BEFORE_OSD_MODULE);
	/* enable osd*/    
	osd_open(WINDOWS0);
	/* enable lcd */	
	//lcd_enable(1);
	
	if (mipi_open("KR070LB0S_1024_600") < 0) 
		return;
	/* set cmd for change pixel format to RGB666 */
	buf[0] = 0x3a;
	buf[1] = 0x55;
	mipi_send_ctrl(buf, 2);
	osd_set_bppmode(0, OSD_IMAGE_RGB_BPP16_565);	
/*
	for (i = COLOR_CODE_16BIT_CONFIG1; i < COLOR_CODE_18BIT_CONFIG1; i++) {
		lcd_set_cfgorder(i + 1);
		mipi_set_pixel_format(i);
#ifndef CONFIG_COMPILE_RTL
		udelay(2000000);
#endif
	}
*/
	lcd_set_cfgorder(config_num + 1);
	mipi_set_pixel_format(config_num);
	lcd_enable(1);
}

static void mipi_dsi_test_switchonoff(void)
{
	int i;

	mipi_dsi_test_24burst(1);

	for (i = 0; i < 10; i++) {
		mipi_display_ctrl(i % 2);
#ifndef CONFIG_COMPILE_RTL
		udelay(500000);
#endif
	}
}

static void mipi_dsi_test_switchcmdvideo(void)
{
	int i;
	
	mipi_dsi_test_24burst(1);
#ifndef CONFIG_COMPILE_RTL
	udelay(2000000);
#endif
	for (i = 0; i < 5; i++) {
		mipi_switch_mode(i / 2);	
		mipi_switch_mode((i + 1) / 2);
		lcd_wait_frame();
#ifndef CONFIG_COMPILE_RTL
		udelay(50000);
#endif
	}
}

static void mipi_dsi_test_no_burst(void)
{
	mipi_dsi_test_24burst(1);
	mipi_set_transfer_mode(VIDEO_NON_BURST_WITH_SYNC_PULSES);	
}

static void mipi_dsi_test_hs(void)
{
	uint8_t buf[2600];
#ifndef CONFIG_COMPILE_RTL
	int i;
#endif

	mipi_dsi_test_24burst(0);
#ifndef CONFIG_COMPILE_RTL
	udelay(2000000);
#endif
	
	/* set mipi display in generic interface&lower power mode */
	mipi_switch_mode(MIPI_DSI_CMD_MODE);
	/* Set page_address : 0 ~ 768 */
	buf[0] = 0x2B;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x04;
	buf[4] = 0x00;	
	dsi_send_long_packet(buf, 5);

	/* Set column_address : 0 ~ 1024 */
	buf[0] = 0x2A;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x03;
	buf[4] = 0x00;
	dsi_send_long_packet(buf, 5);
	
	writel(0x67ff, MIPI_DSI_BASE_ADDR + 0x24);
#ifndef CONFIG_COMPILE_RTL
	buf[0] = 0x2C;
	for (i = 0; i < 0x300; i++) {
		buf[3 * i + 1] = 0xFF;
		buf[3 * i + 2] = 0x00;
		buf[3 * i + 3] = 0x00;
	}
	dsi_send_long_packet(buf, 2305);
	
	for (i = 0; i < 0x400; i++) {
		buf[0] = 0x3C;
		dsi_send_long_packet(buf, 2305);
	}
#else
	udelay(10);
	buf[0] = 0x2C;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x05;
	buf[4] = 0x00;
	dsi_send_long_packet(buf, 5);

#endif
	//mipi_switch_mode(MIPI_DSI_VIDEO_MODE);
#if 1
#ifndef CONFIG_COMPILE_RTL
	udelay(500000);
#endif
	mipi_switch_mode(MIPI_DSI_CMD_MODE);                                                                       
#ifndef CONFIG_COMPILE_RTL
	udelay(100000);                                                      
#endif
	buf[0] = 0x29;                                                  
	buf[1] = 0x00;                                                  
	mipi_send_ctrl(buf, 2);
#ifndef CONFIG_COMPILE_RTL
	udelay(500000);
#endif    
	mipi_switch_mode(MIPI_DSI_VIDEO_MODE);	
#endif
}

static void mipi_dsi_test_lp(void)
{
	uint8_t buf[2600];
#ifndef CONFIG_COMPILE_RTL
	int i;
#endif
	mipi_dsi_test_24burst(0);
#ifndef CONFIG_COMPILE_RTL
	udelay(2000000);
#endif	
	/* set mipi display in generic interface&lower power mode */
	mipi_switch_mode(MIPI_DSI_CMD_MODE);
	/* Set page_address : 0 ~ 768 */
	buf[0] = 0x2B;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x04;
	buf[4] = 0x00;	
	dsi_send_long_packet(buf, 5);

	/* Set column_address : 0 ~ 1024 */
	buf[0] = 0x2A;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x03;
	buf[4] = 0x00;
	dsi_send_long_packet(buf, 5);
	
	writel(0x7fff, MIPI_DSI_BASE_ADDR + 0x24);
#ifndef CONFIG_COMPILE_RTL
	buf[0] = 0x2C;
	for (i = 0; i < 0x300; i++) {
		buf[3 * i + 1] = 0xFF;
		buf[3 * i + 2] = 0x00;
		buf[3 * i + 3] = 0x00;
	}
	dsi_send_long_packet(buf, 2305);
	
	for (i = 0; i < 0x400; i++) {
		buf[0] = 0x3C;
		dsi_send_long_packet(buf, 2305);
	}
#else
	udelay(10);
	buf[0] = 0x2C;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x05;
	buf[4] = 0x00;
	dsi_send_long_packet(buf, 5);
#endif
	//mipi_switch_mode(MIPI_DSI_VIDEO_MODE);
#if 1
#ifndef CONFIG_COMPILE_RTL
	udelay(500000);
#endif
	mipi_switch_mode(MIPI_DSI_CMD_MODE);                                                                       
#ifndef CONFIG_COMPILE_RTL
	udelay(100000);
#endif                                                      
	buf[0] = 0x29;                                                  
	buf[1] = 0x00;                                                  
	mipi_send_ctrl(buf, 2);
#ifndef CONFIG_COMPILE_RTL
	udelay(500000);
#endif    
	mipi_switch_mode(MIPI_DSI_VIDEO_MODE);	
#endif
}

int mipi_dsi_test_dbi(char *name, int format)
{
	struct i80_cmd cmd;	
	struct osd_dma dma;
	int fd;

	if(lcd_init(0) || lcd_open("YL_320_240"))
		return -1;

	if(mipi_init(1) < 0)
		return -1;
	
	if (i80_init(0) || i80_open(name))
		return -1;

	osd_dma_init(&dma, LCD_MODE, 1);
	fd = open("ids/osd/240_320.bmp",O_RDONLY);
	if (fd < 0) {
		ids_err("open file err: %d\n", fd);
		return -1;
	}
	if (read(fd, dma.buf[0].vir_addr, dma.buf[0].size) !=
			dma.buf[0].size) {
		ids_err("Lcd read file err\n");
		return -1;
	}
	close(fd);
	/* init osd module */
	osd_common_init(WINDOWS0, LCD_MODE, &dma);
	osd_set_scaler_mode(SCALER_BEFORE_OSD_MODULE);

	if (i80_dev_cfg(NULL, format) < 0) {
		ids_err("I80 cfd err\n");
		return -1;
	}

	osd_open(WINDOWS0);	
	mipi_open("I80 320");
	//module_set_clock("ids0-eitf", "epll", 24000000, 0);
	module_set_clock("ids0-eitf", "epll", 200000000, 0);
	mipi_switch_mode(MIPI_DSI_CMD_MODE);
        writel(0x67ff, MIPI_DSI_BASE_ADDR + 0x24);
        writel((0x3C<<16 )|239, MIPI_DSI_BASE_ADDR + 0x14);
	i80_enable(1);
	
	cmd.num = 0;
	cmd.cmd = 0x0000;
	cmd.type = CMD_NORMAL_AUTO;
	cmd.rs_ctrl = I80_ACTIVE_LOW;
	i80_set_cmd(&cmd);

	
	cmd.num = 1;
	cmd.cmd = 0x002c;
	cmd.type = CMD_NORMAL_AUTO;
	cmd.rs_ctrl = I80_ACTIVE_LOW;
	i80_set_cmd(&cmd);

	lcd_enable(1);	
	

	i80_wait_frame_end();
	return 0;
}

static void mipi_case_test_dispatch(int case_num)
{
	int ret = 0;
	int i;

	switch (case_num) {
		case MIPI_DSI_TEST_CHECKREGS:
			lcd_init(0);
			mipi_init(0);
			if (mipi_reg_check() < 0)
				ret = 1;
			break;
		case MIPI_DSI_TEST_SWITCHONOFF:
			mipi_dsi_test_switchonoff();
			break;
		case MIPI_DSI_TEST_SWITCHCMDVIDEO:
			mipi_dsi_test_switchcmdvideo();
			break;
		case MIPI_DSI_TEST_24BPP_BURST:
			mipi_dsi_test_24burst(1);
			for (i = 0; i < 2; i++)
                		lcd_wait_frame();
			break;
		case MIPI_DSI_TEST_18BPP_BURST:
			mipi_dsi_test_18burst(COLOR_CODE_18BIT_CONFIG1);
			for (i = 0; i < 2; i++)
                		lcd_wait_frame();
			break;
		case MIPI_DSI_TEST_16BPP_BURST:
			mipi_dsi_test_16burst(COLOR_CODE_16BIT_CONFIG1);
			for (i = 0; i < 2; i++)
                		lcd_wait_frame();
			break;
		case MIPI_DSI_TEST_NO_BURST:
			mipi_dsi_test_no_burst();
			for (i = 0; i < 2; i++)
                		lcd_wait_frame();
			break;
		case MIPI_DSI_TEST_18BPP_CMD:
			break;
		case MIPI_DSI_TEST_LP_MODE:
			mipi_dsi_test_lp();
			break;
		case MIPI_DSI_TEST_HS_MODE:
			mipi_dsi_test_hs();
			break;
		case MIPI_DSI_TEST_IRQ:
			break;
		case MIPI_DSI_TEST_HD:
			mipi_dsi_test_24burst_hd();
			for (i = 0; i < 2; i++)
                		lcd_wait_frame();
			break;
		case MIPI_DSI_TEST_18BPP_BURST_CONFIG2:
			 mipi_dsi_test_18burst(COLOR_CODE_18BIT_CONFIG2);
                        for (i = 0; i < 2; i++)
                                lcd_wait_frame();
			break;
		case MIPI_DSI_TEST_16BPP_BURST_CONFIG2:
			mipi_dsi_test_16burst(COLOR_CODE_16BIT_CONFIG2);
			for (i = 0; i < 2; i++)
                		lcd_wait_frame();
			break;
		case MIPI_DSI_TEST_16BPP_BURST_CONFIG3:
			mipi_dsi_test_16burst(COLOR_CODE_16BIT_CONFIG3);
			for (i = 0; i < 2; i++)
                		lcd_wait_frame();
			break;
		case MIPI_DSI_TEST_DBI:
			hx8347_init();  
			rtl_i80_init();
			mipi_dsi_test_dbi("rtl_i80",2);
			for (i = 0; i < 2; i++)
                		lcd_wait_frame();
			break;
		default:
			mipitest_show_usage();
			return;
	}

	printf("%s test %s\n", casename[case_num], result[ret]);
}

int mipitest_main(int argc, char *argv[])
{
	int option_index = 0, c, ret = 0;
	static const char short_options[] = "c:h";
	static const struct option long_options[] = {
		{"case", 1, 0, 'c'},
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0},
	};
	int case_num = -1;

	while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
		switch (c) {
			case 'c':
				case_num = simple_strtoul(optarg, NULL, 10);
				break;
			case 'h':
			default:
				mipitest_show_usage();
				ret = -1;
				goto err;
		}
	}

	if (optind == 1) {   
		mipitest_show_usage();
		ret = -1;        
		goto err;        
	}                   

	mipi_case_test_dispatch(case_num);	
err: 
	optind = 1;
	return ret;
}

int mipitest_rtl(int id)
{
	printf("invoking id: %d\n", id);
	return 0;
}

void mipitest_init(void)
{
	register_testbench("dsi", mipitest_main, mipitest_rtl);
}

testbench_init(mipitest_init);
