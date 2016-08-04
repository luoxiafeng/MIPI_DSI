#include <ids/ids.h>
#include <ids/mipi.h>
#include <asm/io.h>
#include <common.h>
#include <malloc.h>
#include "../common/lcd_param.c"

static dsih_ctrl_t *instance = NULL;
dsih_dpi_video_t video;

static struct reg_check reg[] = {
	REG_CHECK_INIT(R_DSI_HOST_VERSION,			IDS_RDONLY),
	REG_CHECK_INIT(R_DSI_HOST_PWR_UP,			0x00000001),
	REG_CHECK_INIT(R_DSI_HOST_CLK_MGR,			0x0000ffff),
	REG_CHECK_INIT(R_DSI_HOST_DPI_CFG,			0x000007ff),
	REG_CHECK_INIT(R_DSI_HOST_DBI_CFG,			0x00001fff),
	REG_CHECK_INIT(R_DSI_HOST_DBI_CMDSIZE,		0xffffffff),
	REG_CHECK_INIT(R_DSI_HOST_PCKHDL_CFG,		0x0000007f),
	REG_CHECK_INIT(R_DSI_HOST_VID_MODE_CFG,		0x7fffffff),
	REG_CHECK_INIT(R_DSI_HOST_VID_PKT_CFG,		0x7fffffff),
	REG_CHECK_INIT(R_DSI_HOST_CMD_MODE_CFG,		0x00007fff),
	REG_CHECK_INIT(R_DSI_HOST_TMR_LINE_CFG,		0xffffffff),
	REG_CHECK_INIT(R_DSI_HOST_VTIMING_CFG,		0x07ffffff),
	REG_CHECK_INIT(R_DSI_HOST_PHY_TMR_CFG,		0x0fffffff),
	REG_CHECK_INIT(R_DSI_HOST_GEN_HDR,			0x00ffffff),
	REG_CHECK_INIT(R_DSI_HOST_GEN_PLD_DATA,		0xffffffff),
	REG_CHECK_INIT(R_DSI_HOST_CMD_PKT_STATUS,	IDS_RDONLY),
	REG_CHECK_INIT(R_DSI_HOST_TO_CNT_CFG,		0xffffffff),
	REG_CHECK_INIT(R_DSI_HOST_ERROR_ST0,		IDS_RDONLY),
	REG_CHECK_INIT(R_DSI_HOST_ERROR_ST1,		IDS_RDONLY),
	REG_CHECK_INIT(R_DSI_HOST_ERROR_MSK0,		0x001fffff),
	REG_CHECK_INIT(R_DSI_HOST_ERROR_MSK1,		0x0003ffff),
	REG_CHECK_INIT(R_DSI_HOST_PHY_RSTZ,			0x00000007),
	REG_CHECK_INIT(R_DSI_HOST_PHY_IF_CFG,		0x000003ff),
	REG_CHECK_INIT(R_DSI_HOST_PHY_IF_CTRL,		0x000001ff),
	REG_CHECK_INIT(R_DSI_HOST_PHY_STATUS,		IDS_RDONLY),
	REG_CHECK_INIT(R_DSI_HOST_PHY_TST_CRTL0,	0x00000003),
	REG_CHECK_INIT(R_DSI_HOST_PHY_TST_CRTL1,	0x000100ff),
};

int mipi_reg_check(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(reg); i++)
		reg[i].addr = reg[i].addr * 4 + MIPI_DSI_BASE_ADDR;

	if (common_reg_check(reg, ARRAY_SIZE(reg), 0xffffffff) || common_reg_check(reg, ARRAY_SIZE(reg), 0))
		return -1;

	return 0;
}

static uint32_t mipi_read(uint32_t base, uint32_t offset)
{
	return readl(base + offset);
}

static void mipi_write(uint32_t base, uint32_t offset, uint32_t data)
{
	writel(data, base + offset);	
}

static void log_error(const char *message)
{
	ids_err("%s\n", message);
}

void mipi_get_err_status(void)
{
	uint32_t err0 = mipi_dsih_hal_error_status_0(instance, 0xffffffff);
	uint32_t err1 = mipi_dsih_hal_error_status_1(instance, 0xffffffff);

	if (err0 || err1) {
		ids_err("mipi err0 is 0x%x, err1 is 0x%x\n", err0, err1);
		mipi_dsih_dump_register_configuration(instance, 1, NULL, 0);
	}
}

void dsi_send_long_packet(uint8_t *cmd_buffer, unsigned int length)
{                                                                  
	unsigned int i, j, cnt, cIdle,temp;                            

	for(i=0; i < length; i += j){                                 
		temp = 0x00;                                              
		for (j = 0; j < 4 && (j + i) < length; j++){            
			temp |= cmd_buffer[i+j] << (j * 8);                   
		}                                                         
		for(cnt = 0; cnt < 10; cnt ++){                            
			cIdle = readl(MIPI_DSI_BASE_ADDR + R_DSI_HOST_CMD_PKT_STATUS);
			cIdle &= 0x08;                                         
			if(!cIdle) {                                           
				writel(temp, MIPI_DSI_BASE_ADDR + R_DSI_HOST_GEN_PLD_DATA);
				break;                                            
			}                                                      
		}                                                          
	}                                                             
	for(i = 0; i <= 10;i ++){                                     
		cIdle = readl(MIPI_DSI_BASE_ADDR + R_DSI_HOST_CMD_PKT_STATUS);
		cIdle &= 0x02;                                            
		if(!cIdle){                                               
			temp = (length<< 8) | 0x39;                          
			writel(temp, MIPI_DSI_BASE_ADDR + R_DSI_HOST_GEN_HDR); 
			break;                                                
		}                                                         
	}                                                             
	return;                                                       
}                                                                  


void mipi_display_ctrl(int enable)
{
	if (enable) {
		mipi_dsih_shutdown_controller(instance,0);
	} else {
		mipi_dsih_shutdown_controller(instance,1);
	}
}

void mipi_switch_mode(int mode)
{
	if (mode == MIPI_DSI_CMD_MODE)
		mipi_dsih_cmd_mode(instance, 1);
	else
		mipi_dsih_video_mode(instance, 1);
}

void mipi_send_ctrl(uint8_t *buf, int length)
{
	mipi_dsih_dcs_wr_cmd(instance, DSI_TEST_VIDEO_VC, buf, length);
}

int mipi_recv_cmd(uint8_t *buf, int length)
{
	return mipi_dsih_dcs_rd_cmd(instance, DSI_TEST_VIDEO_VC, buf[0], length - 1, &buf[1]);
}

int mipi_set_pixel_format(dsih_color_coding_t format)
{
	video.color_coding = format;
	if (format == COLOR_CODE_18BIT_CONFIG1 ||
		format == COLOR_CODE_18BIT_CONFIG2)
		video.is_18_loosely = 1;

	return mipi_dsih_dpi_video(instance, &video);
}

int mipi_set_transfer_mode(dsih_video_mode_t mode)
{
	video.video_mode = mode;
	return mipi_dsih_dpi_video(instance, &video);	
}

static int mipi_get_screem_params(dsih_dpi_video_t *video, char *name,uint8_t no_of_lanes)
{
	uint32_t rate;
	int i;

	for (i = 0;i < ARRAY_SIZE(panel);i++)
		if (!strcmp(panel[i].name, name))
			break;

	if (i == ARRAY_SIZE(panel))
		return -1;

	rate = lcd_set_pixel_clk(panel[i].mPixelClock * 1000);
	
	//video->no_of_lanes = 4;
	video->no_of_lanes = no_of_lanes;
	video->virtual_channel = 0;
	video->color_coding = COLOR_CODE_24BIT;
	video->byte_clock = rate*24/(no_of_lanes*8)/1000;
	video->video_mode = VIDEO_BURST_WITH_SYNC_PULSES;
	video->receive_ack_packets = 1;
	video->pixel_clock = rate / 1000;
	video->is_18_loosely = 1;
	video->data_en_polarity = 1;
	//video->h_polarity = panel[i].mHSyncPolarity;
	//video->h_polarity = 0;
	video->h_polarity = !panel[i].mHSyncPolarity;
	video->h_active_pixels = panel[i].mHActive;
	video->h_sync_pixels = panel[i].mHSyncPulseWidth;
	video->h_back_porch_pixels = panel[i].mHBackPorch;
	video->h_total_pixels = panel[i].mHActive + panel[i].mHFrontPorch +
					panel[i].mHSyncPulseWidth + panel[i].mHBackPorch;
	video->v_active_lines = panel[i].mVActive;
	//video->v_polarity = panel[i].mVSyncPolarity;
	//video->v_polarity = 0;
	video->v_polarity = !panel[i].mVSyncPolarity;
	video->v_sync_lines = panel[i].mVSyncPulseWidth;
	video->v_back_porch_lines = panel[i].mVBackPorch;
	video->v_total_lines = panel[i].mVActive + panel[i].mVFrontPorch +
					panel[i].mVSyncPulseWidth + panel[i].mVBackPorch;

	return 0;	
}

int mipi_get_screen_size(int *width, int *height)
{
	*width = video.h_active_pixels;
	*height = video.v_active_lines;
	return 0;
}

int mipi_open(char *name)
{
	//uint8_t cmd_buffer[10];
	uint32_t status_1;
	uint8_t buf;
	int ret;
	uint8_t no_of_lanes;
	uint32_t status;
	uint32_t ref_clock=0;
        //动态分配内存
	if (!instance) 
		instance = malloc(sizeof(dsih_ctrl_t));
        //获取时钟
	ref_clock = module_get_clock("mipi-ref");	

	/* init dsi controller */
	instance->address = MIPI_DSI_BASE_ADDR;//0x22040000
	instance->max_lanes = 4;//最大有4个lane
	instance->max_hs_to_lp_cycles = 100;//从高速到低速所需要的字节时钟周期数
	instance->max_lp_to_hs_cycles = 40;//从低速到高速所需要的字节时钟周期数
	instance->max_bta_cycles = 4095;//总线转换方向所需要的最多时钟周期
	instance->color_mode_polarity = 1;
	instance->shut_down_polarity = 1;
	instance->status = NOT_INITIALIZED;
	instance->core_read_function = mipi_read;
	instance->core_write_function = mipi_write;
	instance->log_error = log_error;
	instance->log_info = printf;
	/* init phy */
	instance->phy_instance.address = MIPI_DSI_BASE_ADDR;
	//instance->phy_instance.reference_freq = 24000;
	instance->phy_instance.reference_freq = ref_clock/1000;
	printf("phy reference clock %d\n",instance->phy_instance.reference_freq);
	instance->phy_instance.status = NOT_INITIALIZED;
	instance->phy_instance.core_read_function = instance->core_read_function;
	instance->phy_instance.core_write_function = instance->core_write_function;
	instance->phy_instance.log_error = instance->log_error;
	instance->phy_instance.log_info = instance->log_info;
	
#ifndef CONFIG_COMPILE_RTL
	udelay(35000);
#endif
	pads_set_mode(72, PADS_MODE_OUT, 1);
	
	ret = mipi_dsih_open(instance);	
	if (ret < 0) {
		ids_err("mipi dsi open err\n");
		return -1;
	}
	no_of_lanes = 2;
	mipi_get_screem_params(&video, name,no_of_lanes);
	ret = mipi_dsih_dpi_video(instance, &video);
	if (ret < 0) {
		ids_err("mipi dsih dpi video failed\n");
		return -1;
	}

	mipi_dsih_shutdown_controller(instance, 0);
#ifndef CONFIG_COMPILE_RTL
	udelay(4000);
#endif
	mipi_dsih_dphy_open(&instance->phy_instance);
#ifndef CONFIG_COMPILE_RTL
	udelay(3000);
#endif
	mipi_dsih_dphy_configure(&instance->phy_instance,
						video.no_of_lanes, video.byte_clock * 8);
	
	status_1 = mipi_dsih_hal_error_status_1(instance, 0xffffffff);
	ids_err("Status_1 = 0x%X\n", status_1);                       

	mipi_dsih_dphy_if_control(&instance->phy_instance, 1);	
#if 1
	buf = 0x11;
	mipi_dsih_dcs_wr_cmd(instance, DSI_TEST_VIDEO_VC, &buf, 1);
#ifndef CONFIG_COMPILE_RTL
	udelay(1000);
#endif
	buf = 0x29;
	mipi_dsih_dcs_wr_cmd(instance, DSI_TEST_VIDEO_VC, &buf, 1);
#else
	cmd_buffer[0] = 0xF0;                                                    
	cmd_buffer[1] = 0x55;                                                    
	cmd_buffer[2] = 0xAA;                                                    
	cmd_buffer[3] = 0x52;                                                    
	cmd_buffer[4] = 0x08;                                                    
	cmd_buffer[5] = 0x00;                                                    
	dsi_send_long_packet(cmd_buffer,6);                                   
	udelay(100000);                                                        
	                                                                         
	cmd_buffer[0] = 0xB1;                                                    
	cmd_buffer[1] = 0xFF;                                                    
	cmd_buffer[2] = 0x00;                                                    
	 dsi_send_long_packet(cmd_buffer,3);                                   
	udelay(100000);                                                        
	                                                                         
	cmd_buffer[0] = 0xBC;                                                    
	cmd_buffer[1] = 0x05;                                                    
	cmd_buffer[2] = 0x05;                                                    
	cmd_buffer[3] = 0x05;                                                    
	dsi_send_long_packet(cmd_buffer,4);                                   
	udelay(100000);                                                        

	cmd_buffer[0] = 0xF0;                                                  
	cmd_buffer[1] = 0x55;                                                  
	cmd_buffer[2] = 0xAA;                                                  
	cmd_buffer[3] = 0x52;                                                  
	cmd_buffer[4] = 0x08;                                                  
	cmd_buffer[5] = 0x01;                                                  
	dsi_send_long_packet(cmd_buffer,6);                                 
	udelay(100000);                                                      

	cmd_buffer[0] = 0x11;                                                  
	cmd_buffer[1] = 0x00;                                                  
	mipi_dsih_gen_wr_cmd(instance, DSI_TEST_VIDEO_VC, 0x05, cmd_buffer, 2);
	udelay(500000);                                                      
	cmd_buffer[0] = 0x29;                                                  
	cmd_buffer[1] = 0x00;                                                  
	mipi_dsih_gen_wr_cmd(instance, DSI_TEST_VIDEO_VC, 0x05, cmd_buffer, 2);
	udelay(200000);                                                      

#endif
	mipi_dsih_dphy_if_control(&instance->phy_instance, 1);
#ifndef CONFIG_COMPILE_RTL
	udelay(1000);
#endif
	/* enter video mode */
	if (!mipi_dsih_hal_gen_is_video_mode(instance)) {
		mipi_dsih_hal_gen_cmd_mode_en(instance, 0);
		mipi_dsih_hal_dpi_video_mode_en(instance, 1);
	}
	//phy status bit4 stop state
	status = 0;
	while(1){
		status = mipi_dsih_dphy_status(&instance->phy_instance,1<<4);
		printf("status %d\n",status);
		if(status)
			break;
		udelay(1);
	}
	return 0;
}

int mipi_close(void)
{
	mipi_display_ctrl(0);	
	free(instance);

	mipi_dsih_shutdown_phy(instance, 1);
	mipi_dsih_close(instance);          
	instance = NULL;
	return 0;
}

/*
*(1)将一个gpio设置为输出模式
*(2)配置HDMI的时钟
*/
int mipi_init(int dbi_flag)
{
	uint32_t val;
	
	/* set mipi reset pulldown */    
	pads_set_mode(72, PADS_MODE_OUT, 0);

	//module_enable("mipi-csi");
	/* for reset dsi and clk gate
	 * enable dsi ref&cfg clock*/
	
	writel(0x7, 0x21e2c01c);
	if(dbi_flag==1){//如果标志是1，则执行这段代码。0x21E0_0000系统控制器，0X2C000是IDS1的偏移
	   val = readl(0x21e2c000);
	   val |= 0x8;
	   writel(val, 0x21e2c000);
	   udelay(10);
	   writel(0,0x21e2c000);
	   
           val = readl(0x21e2c028);
	   val |= 0x01;
           writel(val, 0x21e2c028); 
	}
    	//if(dbi_flag==1){
	//	module_set_clock("ids0-eitf", "epll", 30000000, 0);
	//}
	/* set clock 24M */
    module_set_clock("mipi-cfg", "epll", 24000000, 0);
    module_set_clock("mipi-ref", "epll", 24000000, 0);
	
	return 0;
}
