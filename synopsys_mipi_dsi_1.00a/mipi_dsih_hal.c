/**
 * @file mipi_dsih_hal.c
 * @brief Hardware Abstraction Level of DWC MIPI DSI HOST controller
 *
 *  Synopsys Inc.
 *  SG DWC PT02
 */
#include "mipi_dsih_hal.h"


void mipi_dsih_hal_max_rd_size_tx_type(dsih_ctrl_t * instance, int lp);


/**
 * Write a 32-bit word to the DSI Host core
 * @param instance pointer to structure holding the DSI Host core information
 * @param reg_address register offset in core
 * @param data 32-bit word to be written to register
 */
void mipi_dsih_write_word(dsih_ctrl_t * instance, uint32_t reg_address, uint32_t data)
{

	instance->core_write_function(instance->address, reg_address, data);
}
/**
 * Write a bit field o a 32-bit word to the DSI Host core
 * @param instance pointer to structure holding the DSI Host core information
 * @param reg_address register offset in core
 * @param data to be written to register
 * @param shift bit shift from the left (system is BIG ENDIAN)
 * @param width of bit field
 */
void mipi_dsih_write_part(dsih_ctrl_t * instance, uint32_t reg_address, uint32_t data, uint8_t shift, uint8_t width)
{
	uint32_t mask = (1 << width) - 1;
	uint32_t temp = mipi_dsih_read_word(instance, reg_address);

	temp &= ~(mask << shift);
	temp |= (data & mask) << shift;
	mipi_dsih_write_word(instance, reg_address, temp);
}
/**
 * Write a 32-bit word to the DSI Host core
 * @param instance pointer to structure holding the DSI Host core information
 * @param reg_address offset of register
 * @return 32-bit word value stored in register
 */
uint32_t mipi_dsih_read_word(dsih_ctrl_t * instance, uint32_t reg_address)
{
	return instance->core_read_function(instance->address, reg_address);
}
/**
 * Write a 32-bit word to the DSI Host core
 * @param instance pointer to structure holding the DSI Host core information
 * @param reg_address offset of register in core
 * @param shift bit shift from the left (system is BIG ENDIAN)
 * @param width of bit field
 * @return bit field read from register
 */
uint32_t mipi_dsih_read_part(dsih_ctrl_t * instance, uint32_t reg_address, uint8_t shift, uint8_t width)
{
	return (mipi_dsih_read_word(instance, reg_address) >> shift) & ((1 << width) - 1);
}
/**
 * Get DSI Host core version
 * @param instance pointer to structure holding the DSI Host core information
 * @return ascii number of the version
 */
uint32_t mipi_dsih_hal_get_version(dsih_ctrl_t * instance)
{
	return mipi_dsih_read_word(instance, R_DSI_HOST_VERSION);
}
/**
 * Modify power status of DSI Host core
 * @param instance pointer to structure holding the DSI Host core information
 * @param on (1) or off (0)
 */
void mipi_dsih_hal_power(dsih_ctrl_t * instance, int on)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_PWR_UP, on, 0, 1);
}
/**
 * Get the power status of the DSI Host core
 * @param instance pointer to structure holding the DSI Host core information
 * @return power status
 */
int mipi_dsih_hal_get_power(dsih_ctrl_t * instance)
{
	return (int)(mipi_dsih_read_word(instance, R_DSI_HOST_PWR_UP));
}
/**
 * Write transmission escape timeout
 * a safe guard so that the state machine would reset if transmission
 * takes too long
 * @param instance pointer to structure holding the DSI Host core information
 * @param tx_escape_division
 */
void mipi_dsih_hal_tx_escape_timeout(dsih_ctrl_t * instance, uint8_t tx_escape_division)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_CLK_MGR, tx_escape_division, 0, 8);
}
/**
 * Write the DPI video virtual channel destination
 * @param instance pointer to structure holding the DSI Host core information
 * @param vc virtual channel
 */
void mipi_dsih_hal_dpi_video_vc(dsih_ctrl_t * instance, uint8_t vc)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_DPI_CFG, (uint32_t)(vc), 0, 2);
}
/**
 * Get the DPI video virtual channel destination
 * @param instance pointer to structure holding the DSI Host core information
 * @return virtual channel
 */
uint8_t mipi_dsih_hal_dpi_get_video_vc(dsih_ctrl_t * instance)
{
	return mipi_dsih_read_part(instance, R_DSI_HOST_DPI_CFG, 0, 2);
}
/**
 * Set DPI video color coding
 * @param instance pointer to structure holding the DSI Host core information
 * @param color_coding enum (configuration and color depth)
 * @return error code
 */
dsih_error_t mipi_dsih_hal_dpi_color_coding(dsih_ctrl_t * instance, dsih_color_coding_t color_coding)
{
	dsih_error_t err = OK;
	if (color_coding > 7)
	{
		if (instance->log_error != 0)
		{
			instance->log_error("invalid colour configuration");
		}
		err = ERR_DSI_COLOR_CODING;
	}
	else
	{
		mipi_dsih_write_part(instance, R_DSI_HOST_DPI_CFG, color_coding, 2, 3);
	}
	return err;
}
/**
 * Get DPI video color coding
 * @param instance pointer to structure holding the DSI Host core information
 * @return color coding enum (configuration and color depth)
 */
dsih_color_coding_t mipi_dsih_hal_dpi_get_color_coding(dsih_ctrl_t * instance)
{
	return (dsih_color_coding_t)(mipi_dsih_read_part(instance, R_DSI_HOST_DPI_CFG, 2, 3));
}
/**
 * Get DPI video color depth
 * @param instance pointer to structure holding the DSI Host core information
 * @return number of bits per pixel
 */
uint8_t mipi_dsih_hal_dpi_get_color_depth(dsih_ctrl_t * instance)
{
	uint8_t color_depth = 0;
	switch (mipi_dsih_read_part(instance, R_DSI_HOST_DPI_CFG, 2, 3))
	{
		case 0:
		case 1:
		case 2:
			color_depth = 16;
			break;
		case 3:
		case 4:
			color_depth = 18;
			break;
		default:
			color_depth = 24;
			break;
	}
	return color_depth;
}
/**
 * Get DPI video pixel configuration
 * @param instance pointer to structure holding the DSI Host core information
 * @return pixel configuration
 */
uint8_t mipi_dsih_hal_dpi_get_color_config(dsih_ctrl_t * instance)
{
	uint8_t color_config = 0;
	switch (mipi_dsih_read_part(instance, R_DSI_HOST_DPI_CFG, 2, 3))
	{
		case 0:
			color_config = 1;
		case 1:
			color_config = 2;
			break;
		case 2:
			color_config = 3;
			break;
		case 3:
			color_config = 1;
			break;
		case 4:
			color_config = 2;
			break;
		default:
			color_config = 0;
			break;
	}
	return color_config;
}
/**
 * Set DPI loosely packetisation video (used only when color depth = 18
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable
 */
void mipi_dsih_hal_dpi_18_loosely_packet_en(dsih_ctrl_t * instance, int enable)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_DPI_CFG, enable, 10, 1);
}
/**
 * Set DPI color mode pin polarity
 * @param instance pointer to structure holding the DSI Host core information
 * @param active_low (1) or active high (0)
 */
void mipi_dsih_hal_dpi_color_mode_pol(dsih_ctrl_t * instance, int active_low)
{
	//这里的意思就是将寄存器0xc的 bit9宽度为1，设置为active_low的值。
	//就是设置color mode的极性
	mipi_dsih_write_part(instance, R_DSI_HOST_DPI_CFG, active_low, 9, 1);
}
/**
 * Set DPI shut down pin polarity
 * @param instance pointer to structure holding the DSI Host core information
 * @param active_low (1) or active high (0)
 */
void mipi_dsih_hal_dpi_shut_down_pol(dsih_ctrl_t * instance, int active_low)
{
	//这里的意思就是将寄存器0xc的 bit9宽度为1，设置为active_low的值。
	//就是设置color mode的极性
	mipi_dsih_write_part(instance, R_DSI_HOST_DPI_CFG, active_low, 8, 1);
}
/**
 * Set DPI horizontal sync pin polarity
 * @param instance pointer to structure holding the DSI Host core information
 * @param active_low (1) or active high (0)
 */
void mipi_dsih_hal_dpi_hsync_pol(dsih_ctrl_t * instance, int active_low)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_DPI_CFG, active_low, 7, 1);
}
/**
 * Set DPI vertical sync pin polarity
 * @param instance pointer to structure holding the DSI Host core information
 * @param active_low (1) or active high (0)
 */
void mipi_dsih_hal_dpi_vsync_pol(dsih_ctrl_t * instance, int active_low)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_DPI_CFG, active_low, 6, 1);
}
/**
 * Set DPI data enable pin polarity
 * @param instance pointer to structure holding the DSI Host core information
 * @param active_low (1) or active high (0)
 */
void mipi_dsih_hal_dpi_dataen_pol(dsih_ctrl_t * instance, int active_low)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_DPI_CFG, active_low, 5, 1);
}
/**
 * Enable FRAME BTA ACK
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable (1) - disable (0)
 */
void mipi_dsih_hal_dpi_frame_ack_en(dsih_ctrl_t * instance, int enable)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_VID_MODE_CFG, enable, 11, 1);
}
/**
 * Enable null packets (value in null packet size will be taken in calculations)
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable (1) - disable (0)
 */
void mipi_dsih_hal_dpi_null_packet_en(dsih_ctrl_t * instance, int enable)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_VID_MODE_CFG, enable, 10, 1);
}
/**
 * Enable multi packets (value in no of chunks will be taken in calculations)
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable (1) - disable (0)
 */
void mipi_dsih_hal_dpi_multi_packet_en(dsih_ctrl_t * instance, int enable)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_VID_MODE_CFG, enable, 9, 1);
}
/**
 * Enable return to low power mode inside horizontal front porch periods when
 *  timing allows
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable (1) - disable (0)
 */
void mipi_dsih_hal_dpi_lp_during_hfp(dsih_ctrl_t * instance, int enable)
{
	//在时序允许的情况下，允许在右边界时间周期内，进入到低功耗模式
	//就是将视频模式配置寄存器的对应的bit位写1：VID_MOD_CFG
	mipi_dsih_write_part(instance, R_DSI_HOST_VID_MODE_CFG, enable, 8, 1);
}
/**
 * Enable return to low power mode inside horizontal back porch periods when
 *  timing allows
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable (1) - disable (0)
 */
void mipi_dsih_hal_dpi_lp_during_hbp(dsih_ctrl_t * instance, int enable)
{
	//在时序允许的情况下，允许在左边界时间周期内，进入到低功耗模式
	//就是将视频模式配置寄存器的对应的bit位写1：VID_MOD_CFG
	mipi_dsih_write_part(instance, R_DSI_HOST_VID_MODE_CFG, enable, 7, 1);
}
/**
 * Enable return to low power mode inside vertical active lines periods when
 *  timing allows
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable (1) - disable (0)
 */
void mipi_dsih_hal_dpi_lp_during_vactive(dsih_ctrl_t * instance, int enable)
{
	//在时序允许的情况下，允许在垂直分辨率时间周期内，进入到低功耗模式
	//就是将视频模式配置寄存器的对应的bit位写1：VID_MOD_CFG
	mipi_dsih_write_part(instance, R_DSI_HOST_VID_MODE_CFG, enable, 6, 1);
}
/**
 * Enable return to low power mode inside vertical front porch periods when
 *  timing allows
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable (1) - disable (0)
 */
void mipi_dsih_hal_dpi_lp_during_vfp(dsih_ctrl_t * instance, int enable)
{
	//在时序允许的情况下，允许在下边界时间周期内，进入到低功耗模式
	mipi_dsih_write_part(instance, R_DSI_HOST_VID_MODE_CFG, enable, 5, 1);
}
/**
 * Enable return to low power mode inside vertical back porch periods when
 * timing allows
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable (1) - disable (0)
 */
void mipi_dsih_hal_dpi_lp_during_vbp(dsih_ctrl_t * instance, int enable)
{
	//在时序允许的情况下，允许在上边界时间周期内，进入到低功耗模式
	mipi_dsih_write_part(instance, R_DSI_HOST_VID_MODE_CFG, enable, 4, 1);
}
/**
 * Enable return to low power mode inside vertical sync periods when
 *  timing allows
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable (1) - disable (0)
 */
void mipi_dsih_hal_dpi_lp_during_vsync(dsih_ctrl_t * instance, int enable)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_VID_MODE_CFG, enable, 3, 1);
}
/**
 * Set DPI video mode type (burst/non-burst - with sync pulses or events)
 * @param instance pointer to structure holding the DSI Host core information
 * @param type
 * @return error code
 */
dsih_error_t mipi_dsih_hal_dpi_video_mode_type(dsih_ctrl_t * instance, dsih_video_mode_t type)
{
	if (type < 3)
	{
		mipi_dsih_write_part(instance, R_DSI_HOST_VID_MODE_CFG, type, 1, 2);
		return OK;
	}
	else
	{
		if (instance->log_error != 0)
		{
			instance->log_error("undefined type");
		}
		return ERR_DSI_OUT_OF_BOUND;
	}
}
/**
 * Enable/disable DPI video mode
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable (1) - disable (0)
 */
void mipi_dsih_hal_dpi_video_mode_en(dsih_ctrl_t * instance, int enable)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_VID_MODE_CFG, enable, 0, 1);
}
/**
 * Get the status of video mode, whether enabled or not in core
 * @param instance pointer to structure holding the DSI Host core information
 * @return status
 */
int mipi_dsih_hal_dpi_is_video_mode(dsih_ctrl_t * instance)
{
	return mipi_dsih_read_part(instance, R_DSI_HOST_VID_MODE_CFG, 0, 1);
}
/**
 * Write the null packet size - will only be taken into account when null
 * packets are enabled.
 * @param instance pointer to structure holding the DSI Host core information
 * @param size of null packet
 * @return error code
 */
dsih_error_t mipi_dsih_hal_dpi_null_packet_size(dsih_ctrl_t * instance, uint16_t size)
{
	if (size < 0x3ff) /* 10-bit field */
	{
		mipi_dsih_write_part(instance, R_DSI_HOST_VID_PKT_CFG, size, 21, 10);
		return OK;
	}
	else
	{
		return ERR_DSI_OUT_OF_BOUND;
	}
}
/**
 * Write no of chunks to core - taken into consideration only when multi packet
 * is enabled
 * @param instance pointer to structure holding the DSI Host core information
 * @param no of chunks
 */
dsih_error_t mipi_dsih_hal_dpi_chunks_no(dsih_ctrl_t * instance, uint16_t no)
{
	if (no < 0x3ff)
	{
		mipi_dsih_write_part(instance, R_DSI_HOST_VID_PKT_CFG, no, 11, 10);
		return OK;
	}
	else
	{
		return ERR_DSI_OUT_OF_BOUND;
	}
}
/**
 * Write video packet size. obligatory for sending video
 * @param instance pointer to structure holding the DSI Host core information
 * @param size of video packet - containing information
 * @return error code
 */
dsih_error_t mipi_dsih_hal_dpi_video_packet_size(dsih_ctrl_t * instance, uint16_t size)
{
	if (size < 0x7ff) /* 11-bit field */
	{
		mipi_dsih_write_part(instance, R_DSI_HOST_VID_PKT_CFG, size, 0, 11);
		return OK;
	}
	else
	{
		return ERR_DSI_OUT_OF_BOUND;
	}
}
/**
 * Enable tear effect acknowledge
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable (1) - disable (0)
 */
void mipi_dsih_hal_tear_effect_ack_en(dsih_ctrl_t * instance, int enable)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_CMD_MODE_CFG, enable, 14, 1);
}
/**
 * Enable packets acknowledge request after each packet transmission
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable (1) - disable (0)
 */
void mipi_dsih_hal_cmd_ack_en(dsih_ctrl_t * instance, int enable)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_CMD_MODE_CFG, enable, 13, 1);
}
/**
 * Set DCS command packet transmission to transmission type
 * @param instance pointer to structure holding the DSI Host core information
 * @param no_of_param of command
 * @param lp transmit in low power
 * @return error code
 */
dsih_error_t mipi_dsih_hal_dcs_wr_tx_type(dsih_ctrl_t * instance, unsigned no_of_param, int lp)
{
	switch (no_of_param)
	{
		case 0:
			mipi_dsih_write_part(instance, R_DSI_HOST_CMD_MODE_CFG, lp, 7, 1);
			break;
		case 1:
			mipi_dsih_write_part(instance, R_DSI_HOST_CMD_MODE_CFG, lp, 8, 1);
			break;
		default:
			mipi_dsih_write_part(instance, R_DSI_HOST_CMD_MODE_CFG, lp, 12, 1);
			break;
	}
	return OK;
}
/**
 * Set DCS read command packet transmission to transmission type
 * @param instance pointer to structure holding the DSI Host core information
 * @param no_of_param of command
 * @param lp transmit in low power
 * @return error code
 */
dsih_error_t mipi_dsih_hal_dcs_rd_tx_type(dsih_ctrl_t * instance, unsigned no_of_param, int lp)
{
	dsih_error_t err = OK;
	switch (no_of_param)
	{
		case 0:
			mipi_dsih_write_part(instance, R_DSI_HOST_CMD_MODE_CFG, lp, 9, 1);
			break;
		default:
			if (instance->log_error != 0)
			{
				instance->log_error("undefined DCS Read packet type");
			}
			err = ERR_DSI_OUT_OF_BOUND;
			break;
	}
	return err;
}
/**
 * Set generic write command packet transmission to transmission type
 * @param instance pointer to structure holding the DSI Host core information
 * @param no_of_param of command
 * @param lp transmit in low power
 * @return error code
 */
dsih_error_t mipi_dsih_hal_gen_wr_tx_type(dsih_ctrl_t * instance, unsigned no_of_param, int lp)
{
	switch (no_of_param)
	{
		case 0:
			mipi_dsih_write_part(instance, R_DSI_HOST_CMD_MODE_CFG, lp, 1, 1);
			break;
		case 1:
			mipi_dsih_write_part(instance, R_DSI_HOST_CMD_MODE_CFG, lp, 2, 1);
			break;
		case 2:
			mipi_dsih_write_part(instance, R_DSI_HOST_CMD_MODE_CFG, lp, 3, 1);
			break;
		default:
			mipi_dsih_write_part(instance, R_DSI_HOST_CMD_MODE_CFG, lp, 11, 1);
			break;
	}
	return OK;
}
/**
 * Set generic command packet transmission to transmission type
 * @param instance pointer to structure holding the DSI Host core information
 * @param no_of_param of command
 * @param lp transmit in low power
 * @return error code
 */
dsih_error_t mipi_dsih_hal_gen_rd_tx_type(dsih_ctrl_t * instance, unsigned no_of_param, int lp)
{
	dsih_error_t err = OK;
	switch (no_of_param)
	{
		case 0:
			mipi_dsih_write_part(instance, R_DSI_HOST_CMD_MODE_CFG, lp, 4, 1);
			break;
		case 1:
			mipi_dsih_write_part(instance, R_DSI_HOST_CMD_MODE_CFG, lp, 5, 1);
			break;
		case 2:
			mipi_dsih_write_part(instance, R_DSI_HOST_CMD_MODE_CFG, lp, 6, 1);
			break;
		default:
			if (instance->log_error != 0)
			{
				instance->log_error("undefined Generic Read packet type");
			}
			err = ERR_DSI_OUT_OF_BOUND;
			break;
	}
	return err;
}
/**
 * Configure maximum read packet size command transmission type
 * @param instance pointer to structure holding the DSI Host core information
 * @param lp set to low power
 */
void mipi_dsih_hal_max_rd_size_tx_type(dsih_ctrl_t * instance, int lp)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_CMD_MODE_CFG, lp, 10, 1);
}
/**
 * Enable command mode (Generic interface)
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable
 */
void mipi_dsih_hal_gen_cmd_mode_en(dsih_ctrl_t * instance, int enable)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_CMD_MODE_CFG, enable, 0, 1);
}
/**
 * Retrieve the controller's status of whether command mode is ON or not
 * @param instance pointer to structure holding the DSI Host core information
 * @return whether command mode is ON
 */
int mipi_dsih_hal_gen_is_cmd_mode(dsih_ctrl_t * instance)
{
	return mipi_dsih_read_part(instance, R_DSI_HOST_CMD_MODE_CFG, 0, 1);
}

int mipi_dsih_hal_gen_is_video_mode(dsih_ctrl_t * instance)
{
	return mipi_dsih_read_part(instance, R_DSI_HOST_VID_MODE_CFG, 0, 1);
}
/**
 * Configure the Horizontal Line time
 * @param instance pointer to structure holding the DSI Host core information
 * @param time taken to transmit the total of the horizontal line
 */
void mipi_dsih_hal_dpi_hline(dsih_ctrl_t * instance, uint16_t time)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_TMR_LINE_CFG, time, 18, 14);
}
/**
 * Configure the Horizontal back porch time
 * @param instance pointer to structure holding the DSI Host core information
 * @param time taken to transmit the horizontal back porch
 */
void mipi_dsih_hal_dpi_hbp(dsih_ctrl_t * instance, uint16_t time)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_TMR_LINE_CFG, time, 9, 9);
}
/**
 * Configure the Horizontal sync time
 * @param instance pointer to structure holding the DSI Host core information
 * @param time taken to transmit the horizontal sync
 */
void mipi_dsih_hal_dpi_hsa(dsih_ctrl_t * instance, uint16_t time)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_TMR_LINE_CFG, time, 0, 9);
}
/**
 * Configure the vertical active lines of the video stream
 * @param instance pointer to structure holding the DSI Host core information
 * @param lines
 */
void mipi_dsih_hal_dpi_vactive(dsih_ctrl_t * instance, uint16_t lines)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_VTIMING_CFG, lines, 16, 11);
}
/**
 * Configure the vertical front porch lines of the video stream
 * @param instance pointer to structure holding the DSI Host core information
 * @param lines
 */
void mipi_dsih_hal_dpi_vfp(dsih_ctrl_t * instance, uint16_t lines)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_VTIMING_CFG, lines, 10, 6);
}
/**
 * Configure the vertical back porch lines of the video stream
 * @param instance pointer to structure holding the DSI Host core information
 * @param lines
 */
void mipi_dsih_hal_dpi_vbp(dsih_ctrl_t * instance, uint16_t lines)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_VTIMING_CFG, lines, 4, 6);
}
/**
 * Configure the vertical sync lines of the video stream
 * @param instance pointer to structure holding the DSI Host core information
 * @param lines
 */
void mipi_dsih_hal_dpi_vsync(dsih_ctrl_t * instance, uint16_t lines)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_VTIMING_CFG, lines, 0, 4);
}
/**
 * configure timeout divisions (so they would have more clock ticks)
 * @param instance pointer to structure holding the DSI Host core information
 * @param byte_clk_division_factor
 */
void mipi_dsih_hal_timeout_clock_division(dsih_ctrl_t * instance, uint8_t byte_clk_division_factor)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_CLK_MGR, byte_clk_division_factor, 8, 8);
}
/**
 * Configure the Low power receive time out
 * @param instance pointer to structure holding the DSI Host core information
 * @param count (of byte cycles)
 */
void mipi_dsih_hal_lp_rx_timeout(dsih_ctrl_t * instance, uint16_t count)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_TO_CNT_CFG, count, 16, 16);
}
/**
 * Configure a high speed transmission time out7
 * @param instance pointer to structure holding the DSI Host core information
 * @param count (byte cycles)
 */
void mipi_dsih_hal_hs_tx_timeout(dsih_ctrl_t * instance, uint16_t count)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_TO_CNT_CFG, count, 0, 16);
}
/**
 * Get the error 0 interrupt register status
 * @param instance pointer to structure holding the DSI Host core information
 * @param mask the mask to be read from the register
 * @return error status 0 value
 */
uint32_t mipi_dsih_hal_error_status_0(dsih_ctrl_t * instance, uint32_t mask)
{
	return (mipi_dsih_read_word(instance, R_DSI_HOST_ERROR_ST0) & mask);
}
/**
 * Get the error 1 interrupt register status
 * @param instance pointer to structure holding the DSI Host core information
 * @param mask the mask to be read from the register
 * @return error status 1 value
 */
uint32_t mipi_dsih_hal_error_status_1(dsih_ctrl_t * instance, uint32_t mask)
{
	return (mipi_dsih_read_word(instance, R_DSI_HOST_ERROR_ST1) & mask);
}
/**
 * Configure MASK (hiding) of interrupts coming from error 0 source
 * @param instance pointer to structure holding the DSI Host core information
 * @param mask to be written to the register
 */
void mipi_dsih_hal_error_mask_0(dsih_ctrl_t * instance, uint32_t mask)
{
	mipi_dsih_write_word(instance, R_DSI_HOST_ERROR_MSK0, mask);
}
/**
 * Get the ERROR MASK  0 register status
 * @param instance pointer to structure holding the DSI Host core information
 * @param mask the bits to read from the mask register
 */
uint32_t mipi_dsih_hal_get_error_mask_0(dsih_ctrl_t * instance, uint32_t mask)
{
	return (mipi_dsih_read_word(instance, R_DSI_HOST_ERROR_MSK0) & mask);
}
/**
 * Configure MASK (hiding) of interrupts coming from error 0 source
 * @param instance pointer to structure holding the DSI Host core information
 * @param mask the mask to be written to the register
 */
void mipi_dsih_hal_error_mask_1(dsih_ctrl_t * instance, uint32_t mask)
{
	mipi_dsih_write_word(instance, R_DSI_HOST_ERROR_MSK1, mask);
}
/**
 * Get the ERROR MASK  1 register status
 * @param instance pointer to structure holding the DSI Host core information
 * @param mask the bits to read from the mask register
 */
uint32_t mipi_dsih_hal_get_error_mask_1(dsih_ctrl_t * instance, uint32_t mask)
{
	return (mipi_dsih_read_word(instance, R_DSI_HOST_ERROR_MSK1) & mask);
}
/* DBI NOT IMPLEMENTED */
void mipi_dsih_hal_dbi_out_color_coding(dsih_ctrl_t * instance, uint8_t color_depth, uint8_t option);
void mipi_dsih_hal_dbi_in_color_coding(dsih_ctrl_t * instance, uint8_t color_depth, uint8_t option);
void mipi_dsih_hal_dbi_lut_size(dsih_ctrl_t * instance, uint8_t size);
void mipi_dsih_hal_dbi_partitioning_en(dsih_ctrl_t * instance, int enable);
void mipi_dsih_hal_dbi_dcs_vc(dsih_ctrl_t * instance, uint8_t vc);
void mipi_dsih_hal_dbi_max_cmd_size(dsih_ctrl_t * instance, uint16_t size);
void mipi_dsih_hal_dbi_cmd_size(dsih_ctrl_t * instance, uint16_t size);
void mipi_dsih_hal_dbi_max_cmd_size(dsih_ctrl_t * instance, uint16_t size);
int mipi_dsih_hal_dbi_rd_cmd_busy(dsih_ctrl_t * instance);
int mipi_dsih_hal_dbi_read_fifo_full(dsih_ctrl_t * instance);
int mipi_dsih_hal_dbi_read_fifo_empty(dsih_ctrl_t * instance);
int mipi_dsih_hal_dbi_write_fifo_full(dsih_ctrl_t * instance);
int mipi_dsih_hal_dbi_write_fifo_empty(dsih_ctrl_t * instance);
int mipi_dsih_hal_dbi_cmd_fifo_full(dsih_ctrl_t * instance);
int mipi_dsih_hal_dbi_cmd_fifo_empty(dsih_ctrl_t * instance);

/**
 * Write command header in the generic interface
 * (which also sends DCS commands) as a subset
 * @param instance pointer to structure holding the DSI Host core information
 * @param vc of destination
 * @param packet_type (or type of DCS command)
 * @param ls_byte (if DCS, it is the DCS command)
 * @param ms_byte (only parameter of short DCS packet)
 * @return error code
 */
dsih_error_t mipi_dsih_hal_gen_packet_header(dsih_ctrl_t * instance, uint8_t vc, uint8_t packet_type, uint8_t ms_byte, uint8_t ls_byte)
{
	if (vc < 4)
	{
		mipi_dsih_write_part(instance, R_DSI_HOST_GEN_HDR, (ms_byte <<  16) | (ls_byte << 8 ) | ((vc << 6) | packet_type), 0, 24);
		return OK;
	}
	return  ERR_DSI_OVERFLOW;
}
/**
 * Write the payload of the long packet commands
 * @param instance pointer to structure holding the DSI Host core information
 * @param payload array of bytes of payload
 * @return error code
 */
dsih_error_t mipi_dsih_hal_gen_packet_payload(dsih_ctrl_t * instance, uint32_t payload)
{
	if (mipi_dsih_hal_gen_write_fifo_full(instance))
	{
		return ERR_DSI_OVERFLOW;
	}
	mipi_dsih_write_word(instance, R_DSI_HOST_GEN_PLD_DATA, payload);
	return OK;

}
/**
 * Write the payload of the long packet commands
 * @param instance pointer to structure holding the DSI Host core information
 * @param payload pointer to 32-bit array to hold read information
 * @return error code
 */
dsih_error_t  mipi_dsih_hal_gen_read_payload(dsih_ctrl_t * instance, uint32_t* payload)
{
	*payload = mipi_dsih_read_word(instance, R_DSI_HOST_GEN_PLD_DATA);
	return OK;
}

/**
 * Configure the read back virtual channel for the generic interface
 * @param instance pointer to structure holding the DSI Host core information
 * @param vc to listen to on the line
 */
void mipi_dsih_hal_gen_rd_vc(dsih_ctrl_t * instance, uint8_t vc)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_PCKHDL_CFG, vc, 5, 2);
}
/**
 * Enable EOTp reception
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable
 */
void mipi_dsih_hal_gen_eotp_rx_en(dsih_ctrl_t * instance, int enable)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_PCKHDL_CFG, enable, 1, 1);
}
/**
 * Enable EOTp transmission
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable
 */
void mipi_dsih_hal_gen_eotp_tx_en(dsih_ctrl_t * instance, int enable)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_PCKHDL_CFG, enable, 0, 1);
}
/**
 * Enable Bus Turn-around request
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable
 */
void mipi_dsih_hal_bta_en(dsih_ctrl_t * instance, int enable)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_PCKHDL_CFG, enable, 2, 1);
}
/**
 * Enable ECC reception, error correction and reporting
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable
 */
void mipi_dsih_hal_gen_ecc_rx_en(dsih_ctrl_t * instance, int enable)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_PCKHDL_CFG, enable, 3, 1);
}
/**
 * Enable CRC reception, error reporting
 * @param instance pointer to structure holding the DSI Host core information
 * @param enable
 */
void mipi_dsih_hal_gen_crc_rx_en(dsih_ctrl_t * instance, int enable)
{
	mipi_dsih_write_part(instance, R_DSI_HOST_PCKHDL_CFG, enable, 4, 1);
}
/**
 * Get status of read command
 * @param instance pointer to structure holding the DSI Host core information
 * @return 1 if busy
 */
int mipi_dsih_hal_gen_rd_cmd_busy(dsih_ctrl_t * instance)
{
	return mipi_dsih_read_part(instance, R_DSI_HOST_CMD_PKT_STATUS, 6, 1);
}
/**
 * Get the FULL status of generic read payload fifo
 * @param instance pointer to structure holding the DSI Host core information
 * @return 1 if fifo full
 */
int mipi_dsih_hal_gen_read_fifo_full(dsih_ctrl_t * instance)
{
	return mipi_dsih_read_part(instance, R_DSI_HOST_CMD_PKT_STATUS, 5, 1);
}
/**
 * Get the EMPTY status of generic read payload fifo
 * @param instance pointer to structure holding the DSI Host core information
 * @return 1 if fifo empty
 */
int mipi_dsih_hal_gen_read_fifo_empty(dsih_ctrl_t * instance)
{
	return mipi_dsih_read_part(instance, R_DSI_HOST_CMD_PKT_STATUS, 4, 1);
}
/**
 * Get the FULL status of generic write payload fifo
 * @param instance pointer to structure holding the DSI Host core information
 * @return 1 if fifo full
 */
int mipi_dsih_hal_gen_write_fifo_full(dsih_ctrl_t * instance)
{
	return mipi_dsih_read_part(instance, R_DSI_HOST_CMD_PKT_STATUS, 3, 1);
}
/**
 * Get the EMPTY status of generic write payload fifo
 * @param instance pointer to structure holding the DSI Host core information
 * @return 1 if fifo empty
 */
int mipi_dsih_hal_gen_write_fifo_empty(dsih_ctrl_t * instance)
{
	return mipi_dsih_read_part(instance, R_DSI_HOST_CMD_PKT_STATUS, 2, 1);
}
/**
 * Get the FULL status of generic command fifo
 * @param instance pointer to structure holding the DSI Host core information
 * @return 1 if fifo full
 */
int mipi_dsih_hal_gen_cmd_fifo_full(dsih_ctrl_t * instance)
{
	return mipi_dsih_read_part(instance, R_DSI_HOST_CMD_PKT_STATUS, 1, 1);
}
/**
 * Get the EMPTY status of generic command fifo
 * @param instance pointer to structure holding the DSI Host core information
 * @return 1 if fifo empty
 */
int mipi_dsih_hal_gen_cmd_fifo_empty(dsih_ctrl_t * instance)
{
	return mipi_dsih_read_part(instance, R_DSI_HOST_CMD_PKT_STATUS, 0, 1);
}
/* only if DPI */
/**
 * Configure how many cycles of byte clock would the PHY module take
 * to switch from high speed to low power
 * @param instance pointer to structure holding the DSI Host core information
 * @param no_of_byte_cycles
 * @return error code
 */
dsih_error_t mipi_dsih_phy_hs2lp_config(dsih_ctrl_t * instance, uint8_t no_of_byte_cycles)
{
	//这里的意思就是将寄存器0x30的 bit20宽度为8，设置为no_of_byte_cycles的值。
	//就是设置从高速切换到低功耗状态，所需要的时钟cycles
	mipi_dsih_write_part(instance, R_DSI_HOST_PHY_TMR_CFG, no_of_byte_cycles, 20, 8);
	return OK;
}
/**
 * Configure how many cycles of byte clock would the PHY module take
 * to switch from to low power high speed
 * @param instance pointer to structure holding the DSI Host core information
 * @param no_of_byte_cycles
 * @return error code
 */
dsih_error_t mipi_dsih_phy_lp2hs_config(dsih_ctrl_t * instance, uint8_t no_of_byte_cycles)
{
	//这里的意思就是将寄存器0x30的 bit12宽度为8，设置为no_of_byte_cycles的值。
	//就是设置从低功耗状态切换到高速模式，所需要的时钟cycles
	mipi_dsih_write_part(instance, R_DSI_HOST_PHY_TMR_CFG, no_of_byte_cycles, 12, 8);
	return OK;
}
/**
 * Configure how many cycles of byte clock would the PHY module take
 * to turn the bus around to start receiving
 * @param instance pointer to structure holding the DSI Host core information
 * @param no_of_byte_cycles
 * @return error code
 */
dsih_error_t mipi_dsih_phy_bta_time(dsih_ctrl_t * instance, uint16_t no_of_byte_cycles)
{
	if (no_of_byte_cycles < 0x8000) /* 12-bit field */
	{
		//这里的意思就是将寄存器0x30的 bit0宽度为12，设置为no_of_byte_cycles的值。
	    	//就是设置bus 转向，所需要的时钟cycles
		mipi_dsih_write_part(instance, R_DSI_HOST_PHY_TMR_CFG, no_of_byte_cycles, 0, 12);
	}
	else
	{
		return ERR_DSI_OVERFLOW;
	}
	return OK;
}
/* */

