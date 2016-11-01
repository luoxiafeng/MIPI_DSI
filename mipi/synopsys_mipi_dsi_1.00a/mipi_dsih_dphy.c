/**
 * @file mipi_dsih_dphy.c
 * @brief D-PHY driver
 *
 *  Synopsys Inc.
 *  SG DWC PT02
 */
#include "mipi_dsih_dphy.h"

extern dsih_error_t mipi_dsih_wait_pll_lock(void);

/**
 * Initialise D-PHY module and power up
 * @param phy pointer to structure which holds information about the d-phy module
 * @return error code
 */
dsih_error_t mipi_dsih_dphy_open(dphy_t * phy)
{
	if (phy == 0)
	{
		return ERR_DSI_PHY_INVALID;
	}
	else if ((phy->core_read_function == 0) || (phy->core_write_function == 0))
	{
		return ERR_DSI_INVALID_IO;
	}
	else if (phy->status == INITIALIZED)
	{
		return ERR_DSI_PHY_INVALID;
	}
	phy->status = NOT_INITIALIZED;
	mipi_dsih_dphy_reset(phy, 0);
	mipi_dsih_dphy_reset(phy, 1);
	mipi_dsih_dphy_clock_en(phy, 1);
	mipi_dsih_dphy_shutdown(phy, 1);
	mipi_dsih_dphy_stop_wait_time(phy, 1);
	mipi_dsih_dphy_no_of_lanes(phy, 1);
	phy->status = INITIALIZED;
	return OK;
}

void transfer_testcode(dphy_t *phy, uint8_t addr, uint8_t val) {
	/* assert address */
	mipi_dsih_dphy_write_word(phy, R_DSI_HOST_PHY_TST_CRTL1, addr);
	mipi_dsih_dphy_write_word(phy, R_DSI_HOST_PHY_TST_CRTL1, 0x10000 | addr);
	
	/* send */
	mipi_dsih_dphy_write_word(phy, R_DSI_HOST_PHY_TST_CRTL0, 0x02);
	mipi_dsih_dphy_write_word(phy, R_DSI_HOST_PHY_TST_CRTL0, 0x00);

	/* assert value */
	mipi_dsih_dphy_write_word(phy, R_DSI_HOST_PHY_TST_CRTL1, addr);
	mipi_dsih_dphy_write_word(phy, R_DSI_HOST_PHY_TST_CRTL1, val);

	/* send */
	mipi_dsih_dphy_write_word(phy, R_DSI_HOST_PHY_TST_CRTL0, 0x02);
	mipi_dsih_dphy_write_word(phy, R_DSI_HOST_PHY_TST_CRTL0, 0x00);

	/* read */
	printf("0x%x::fb::0x%x\n", addr, (mipi_dsih_dphy_read_word(phy, R_DSI_HOST_PHY_TST_CRTL1) >> 8)
		& 0xff);
}

int calc_pll_para(uint32_t a[], int ref, int freq)
{
	/* parameters in a:
	 * n, m, cp, lpf, range
	 */
	int n, m, cp , lpf, rg, out;

	n = 6;
	m = freq / (ref / n);

	printf("m = %d\n", m);
	//if (m < 12) { cp =0x06; lpf = 0x10; m=60;}//I80 320 low rate dbi case
	if (m < 12) { cp =0x4; lpf = 0x04; m=250;}//I80 320 low rate dbi case
	else if (m <  65) { cp = 0x6; lpf = 0x10; }
	else if (m < 129) { cp = 0xc; lpf = 0x08; }
	else if (m < 257) { cp = 0x4; lpf = 0x04; }
	else if (m < 513) { cp = 0x0; lpf = 0x01; }
	else if (m < 769) { cp = 0x1; lpf = 0x01; }
	else if (m < 999) { cp = 0x2; lpf = 0x01; }
	else return -1;

	out = ref / n * m / 1000;
	printf("out = %d\n", out);

	if (out < 80 || out > 1000) return -1;
	else if (out < 160) rg = 0x1;
	else if (out < 250) rg = 0x3;
	else if (out < 500) rg = 0x7;
	else if (out < 999) rg = 0xf;
	else if (out <= 1500) rg = 0xf;
	else
	     return -1;

	a[0] = n;
	a[1] = m;
	a[2] = cp;
	a[3] = lpf;
	a[4] = rg;

	return out * 1000;
}

int calc_hsfreqranges(int freq)
{
	int value = 0;
	if((freq<80)||(freq>1500))
		return -1;

	if((freq>=80)&&(freq<=89))
		value = 0x00;
	else if((freq>=90)&&(freq<=99))
		value =	0x10;
	else if((freq>=100)&&(freq<=109))
		value =	0x20;
	else if((freq>=110)&&(freq<=129))
		value =	0x01;
	else if((freq>=130)&&(freq<=139))
		value =	0x11;
	else if((freq>=140)&&(freq<=149))
		value =	0x21;
	else if((freq>=150)&&(freq<=169))
		value =	0x02;
	else if((freq>=170)&&(freq<=179))
		value =	0x12;
	else if((freq>=180)&&(freq<=199))
		value =	0x22;
	else if((freq>=200)&&(freq<=219))
		value =	0x03;
	else if((freq>=220)&&(freq<=239))
		value =	0x13;
	else if((freq>=240)&&(freq<=249))
		value =	0x23;
	else if((freq>=250)&&(freq<=269))
		value =	0x04;
	else if((freq>=270)&&(freq<=299))
		value =	0x14;
	else if((freq>=300)&&(freq<=329))
		value = 0x05;	
	else if((freq>=330)&&(freq<=359))
		value = 0x15;	
	else if((freq>=360)&&(freq<=399))
		value =	0x25;
	else if((freq>=400)&&(freq<=449))
		value =	0x06;
	else if((freq>=450)&&(freq<=499))
		value = 0x16;	
	else if((freq>=500)&&(freq<=549))
		value = 0x07;	
	else if((freq>=550)&&(freq<=599))
		value =	0x17;
	else if((freq>=600)&&(freq<=649))
		value =	0x08;
	else if((freq>=650)&&(freq<=699))
		value = 0x18;	
	else if((freq>=700)&&(freq<=749))
		value =	0x09;	
	else if((freq>=750)&&(freq<=799))
		value = 0x19;	
	else if((freq>=800)&&(freq<=849))
		value =	0x29;
	else if((freq>=850)&&(freq<=899))
		value =	0x39;
	else if((freq>=900)&&(freq<=949))
		value =	0x0a;
	else if((freq>=950)&&(freq<=999))
		value = 0x1a;	
	else if((freq>=1000)&&(freq<=1049))
		value =	0x2a;
	else if((freq>=1050)&&(freq<=1099))
		value =	0x3a;
	else if((freq>=1100)&&(freq<=1149))
		value =	0x0b;
	else if((freq>=1150)&&(freq<=1199))
		value =0x1b;
	else if((freq>=1200)&&(freq<=1249))
		value =	0x2b;
	else if((freq>=1250)&&(freq<=1299))
		value =	0x3b;
	else if((freq>=1300)&&(freq<=1349))
		value =	0x0c;
	else if((freq>=1350)&&(freq<=1399))
		value =	0x1c;
	else if((freq>=1400)&&(freq<=1449))
		value =	0x2c;
	else if((freq>=1450)&&(freq<=1500))
		value =	0x3c;
	return value;
}
/**
 * Configure D-PHY module to desired operation mode
 * @param phy pointer to structure which holds information about the d-phy module
 * @param no_of_lanes active
 * @param output_freq desired high speed frequency
 * @return error code
 */
dsih_error_t mipi_dsih_dphy_configure(dphy_t * phy, uint8_t no_of_lanes, uint32_t output_freq)
{
	uint32_t para[8];
	uint32_t freq;
	uint8_t hsfreqranges=0;
	
	if (phy == 0)
	{
		return ERR_DSI_INVALID_INSTANCE;
	}
	if (phy->status < INITIALIZED)
	{
		return ERR_DSI_INVALID_INSTANCE;
	}
#ifdef DWC_MIPI_DPHY_BIDIR_TSMC40LP
	/* for this computations, check databook page 59 */
	/* PLL input divider ratio is not computed since reference_freq < 40MHz */
	divider = (output_freq * 1000/ phy->reference_freq);
	divider /= 1000;
	divider -= 1;
	mipi_dsih_dphy_reset(phy, 1);
	mipi_dsih_dphy_clock_en(phy, 1);
	mipi_dsih_dphy_shutdown(phy, 1);
	no_of_bytes = 2; /* pll loop divider takes only 2 bytes (10 bits in data) */
	for (i = 0; i < no_of_bytes; i++)
	{
		data[i] = ((uint8_t)(divider >> (8 * i)) | (i << 7) );
		/* 7 is dependent on no_of_bytes
		(7 = sizeof(uint8_t) - min_needed_bits to write no_of_bytes)*/
	}
	/* test mode = PLL's loop divider ratio */
	mipi_dsih_dphy_write(phy, 0x18, data, no_of_bytes);

	mipi_dsih_dphy_no_of_lanes(phy, no_of_lanes);
	/*TODO - check */
	//mipi_dsih_dphy_if_control(phy, 1);
	//mipi_dsih_dphy_stop_wait_time(phy, 1);
#endif

/* Mark's */
#ifdef DWC_MIPI_DPHY_BIDIR_2L_RTL
	mipi_dsih_dphy_reset(phy, 1);
	mipi_dsih_dphy_clock_en(phy, 1);
	mipi_dsih_dphy_shutdown(phy, 1);
	mipi_dsih_dphy_no_of_lanes(phy, no_of_lanes);

	mipi_dsih_dphy_write_word(phy, R_DSI_HOST_PHY_RSTZ , 0x04);
	mipi_dsih_dphy_write_word(phy, R_DSI_HOST_PHY_TST_CRTL0, 0x01);
	mipi_dsih_dphy_write_word(phy, R_DSI_HOST_PHY_TST_CRTL0, 0x00);

	printf("using testcode method.\n");

	/* parameters in a:
	 * n, m, cp, lpf, range
	 */
	freq = calc_pll_para(para, phy->reference_freq, output_freq);
	printf("freq %d\n",freq);
	hsfreqranges = calc_hsfreqranges(freq/1000);
	printf("hsfreqranges 0x%08x\n",hsfreqranges);	
//	para[0] = 1; para[1] = 18; para[2] = 6; para[3] = 16; para[4] = 7;
	printf("::: para meters: %x, %x, %x, %x, %x\n",
			para[0], para[1], para[2], para[3], para[4]);

	/* bypass 44 */
	transfer_testcode(phy, 0x44, (hsfreqranges<<1)|0x1);

	transfer_testcode(phy, 0x12, (3 << 6) | para[3]);

	transfer_testcode(phy, 0x10, (1 << 7) | (para[4] << 3));
	transfer_testcode(phy, 0x11, para[2]);

	transfer_testcode(phy, 0x19, 0x30);

	/* n */
	transfer_testcode(phy, 0x17, para[0] - 1);

	/* m */
	transfer_testcode(phy, 0x18, (para[1] - 1) & 0x1f);
	transfer_testcode(phy, 0x18, (1 << 7) | ((para[1] - 1) >> 5));

	transfer_testcode(phy, 0x71, 0x94);

	mipi_dsih_dphy_write_word(phy, R_DSI_HOST_PHY_RSTZ, 0x07);
#endif

/* Sam's */
#ifdef DWC_MIPI_DPHY_TSMC40LP_RTL
	mipi_dsih_dphy_reset(phy, 1);
	mipi_dsih_dphy_clock_en(phy, 1);
	mipi_dsih_dphy_shutdown(phy, 1);
	mipi_dsih_dphy_no_of_lanes(phy, no_of_lanes);

	mipi_dsih_dphy_write_word(phy, R_DSI_HOST_PHY_RSTZ , 0x04);
	mipi_dsih_dphy_write_word(phy, R_DSI_HOST_PHY_TST_CRTL0, 0x01);
	mipi_dsih_dphy_write_word(phy, R_DSI_HOST_PHY_TST_CRTL0, 0x00);

	printf("using testcode method.\n");

	/* parameters in a:
	 * n, m, cp, lpf, range
	 */
	calc_pll_para(para, phy->reference_freq, output_freq);

//	para[0] = 1; para[1] = 18; para[2] = 6; para[3] = 16; para[4] = 7;
	printf("::: para meters: %x, %x, %x, %x, %x\n",
			para[0], para[1], para[2], para[3], para[4]);

	/* bypass 44 */
	transfer_testcode(phy, 0x44, 0x1);

	transfer_testcode(phy, 0x12, (3 << 6) | para[3]);

	transfer_testcode(phy, 0x10, (1 << 7) | (para[4] << 3));
	transfer_testcode(phy, 0x11, para[2]);

	transfer_testcode(phy, 0x19, 0x30);

	/* n */
	transfer_testcode(phy, 0x17, para[0] - 1);

	/* m */
	transfer_testcode(phy, 0x18, (para[1] - 1) & 0x1f);
	transfer_testcode(phy, 0x18, (1 << 7) | ((para[1] - 1) >> 5));

	transfer_testcode(phy, 0x71, 0x94);

	mipi_dsih_dphy_write_word(phy, R_DSI_HOST_PHY_RSTZ, 0x07);
#endif


	mipi_dsih_wait_pll_lock();
	
	return OK;
}
/**
 * Close and power down D-PHY module
 * @param phy pointer to structure which holds information about the d-phy module
 * @return error code
 */
dsih_error_t mipi_dsih_dphy_close(dphy_t * phy)
{
	if (phy == 0)
	{
		return ERR_DSI_INVALID_INSTANCE;
	}
	else if ((phy->core_read_function == 0) || (phy->core_write_function == 0))
	{
		return ERR_DSI_INVALID_IO;
	}
	if (phy->status < NOT_INITIALIZED)
	{
		return ERR_DSI_INVALID_INSTANCE;
	}
	mipi_dsih_dphy_reset(phy, 0);
	mipi_dsih_dphy_reset(phy, 1);
	mipi_dsih_dphy_shutdown(phy, 0);
	phy->status = NOT_INITIALIZED;
	return OK;
}
/**
 * Enable clock lane module
 * @param instance pointer to structure which holds information about the d-phy module
 * @param en
 */
void mipi_dsih_dphy_clock_en(dphy_t * instance, int en)
{
	mipi_dsih_dphy_write_part(instance, R_DSI_HOST_PHY_RSTZ, en, 2, 1);
}
/**
 * Reset D-PHY module
 * @param instance pointer to structure which holds information about the d-phy module
 * @param reset
 */
void mipi_dsih_dphy_reset(dphy_t * instance, int reset)
{
	mipi_dsih_dphy_write_part(instance, R_DSI_HOST_PHY_RSTZ, reset, 1, 1);
}
/**
 * Power up/dpwn D-PHY module
 * @param instance pointer to structure which holds information about the d-phy module
 * @param powerup (1) shutdown (0)
 */
void mipi_dsih_dphy_shutdown(dphy_t * instance, int powerup)
{
	mipi_dsih_dphy_write_part(instance, R_DSI_HOST_PHY_RSTZ, powerup, 0, 1);
}
/**
 * Configure minimum wait period for HS transmission request after a stop state
 * @param instance pointer to structure which holds information about the d-phy module
 * @param no_of_byte_cycles [in byte (lane) clock cycles]
 */
void mipi_dsih_dphy_stop_wait_time(dphy_t * instance, uint8_t no_of_byte_cycles)
{
	mipi_dsih_dphy_write_part(instance, R_DSI_HOST_PHY_IF_CFG, no_of_byte_cycles, 2, 8);
}
/**
 * Set number of active lanes
 * @param instance pointer to structure which holds information about the d-phy module
 * @param no_of_lanes
 */
void mipi_dsih_dphy_no_of_lanes(dphy_t * instance, uint8_t no_of_lanes)
{
	mipi_dsih_dphy_write_part(instance, R_DSI_HOST_PHY_IF_CFG, no_of_lanes - 1, 0, 2);
}
/**
 * Get number of currently active lanes
 * @param instance pointer to structure which holds information about the d-phy module
 * @return number of active lanes
 */
uint8_t mipi_dsih_dphy_get_no_of_lanes(dphy_t * instance)
{
	return mipi_dsih_dphy_read_part(instance, R_DSI_HOST_PHY_IF_CFG, 0, 2);
}
/**
 * D-PHY PPI interface control configuration
 * @param instance pointer to structure which holds information about the d-phy module
 * @param mask
 */
void mipi_dsih_dphy_if_control(dphy_t * instance, uint8_t mask)
{
	mipi_dsih_dphy_write_word(instance, R_DSI_HOST_PHY_IF_CTRL, mask);
}
/**
 * Get set D-PHY PPI interface control configurations
 * @param instance pointer to structure which holds information about the d-phy module
 * @param mask
 * @return value stored in register
 */
uint32_t mipi_dsih_dphy_get_if_control(dphy_t * instance, uint8_t mask)
{
	return (mipi_dsih_dphy_read_word(instance, R_DSI_HOST_PHY_IF_CTRL) & mask);
}
/**
 * Get D-PHY PPI status
 * @param instance pointer to structure which holds information about the d-phy module
 * @param mask
 * @return status
 */
uint32_t mipi_dsih_dphy_status(dphy_t * instance, uint16_t mask)
{
	return mipi_dsih_dphy_read_word(instance, R_DSI_HOST_PHY_STATUS) & mask;
}
/**
 * @param instance pointer to structure which holds information about the d-phy module
 * @param value
 */
void mipi_dsih_dphy_test_clock(dphy_t * instance, int value)
{
	mipi_dsih_dphy_write_part(instance, R_DSI_HOST_PHY_TST_CRTL0, value, 1, 1);
}
/**
 * @param instance pointer to structure which holds information about the d-phy module
 * @param value
 */
void mipi_dsih_dphy_test_clear(dphy_t * instance, int value)
{
	mipi_dsih_dphy_write_part(instance, R_DSI_HOST_PHY_TST_CRTL0, value, 0, 1);
}
/**
 * @param instance pointer to structure which holds information about the d-phy module
 * @param on_falling_edge
 */
void mipi_dsih_dphy_test_en(dphy_t * instance, uint8_t on_falling_edge)
{
	mipi_dsih_dphy_write_part(instance, R_DSI_HOST_PHY_TST_CRTL1, on_falling_edge, 16, 1);
}
/**
 * @param instance pointer to structure which holds information about the d-phy module
 */
uint8_t mipi_dsih_dphy_test_data_out(dphy_t * instance)
{
	return mipi_dsih_dphy_read_part(instance, R_DSI_HOST_PHY_TST_CRTL1, 8, 8);
}
/**
 * @param instance pointer to structure which holds information about the d-phy module
 * @param test_data
 */
void mipi_dsih_dphy_test_data_in(dphy_t * instance, uint8_t test_data)
{
	mipi_dsih_dphy_write_word(instance, R_DSI_HOST_PHY_TST_CRTL1, test_data);
}
/**
 * Write to D-PHY module (encapsulating the digital interface)
 * @param instance pointer to structure which holds information about the d-phy module
 * @param address offset inside the D-PHY digital interface
 * @param data array of bytes to be written to D-PHY
 * @param data_length of the data array
 */
void mipi_dsih_dphy_write(dphy_t * instance, uint8_t address, uint8_t * data, uint8_t data_length)
{
	//unsigned i = 0;
	if (data != 0)
	{
#ifdef DWC_MIPI_DPHY_BIDIR_TSMC40LP
		mipi_dsih_dphy_reset(instance, 1);
		mipi_dsih_dphy_clock_en(instance, 1);
		mipi_dsih_dphy_shutdown(instance, 1);
		/* provide an initial active-high test clear pulse in TESTCLR  */
		mipi_dsih_dphy_test_clear(instance, 1);
		mipi_dsih_dphy_test_clear(instance, 0);
		/* clear the 8-bit input bus to 00h (code for normal operation) */
		mipi_dsih_dphy_test_data_in(instance, 0);
		/* set the TESTCLK input high in preparation to latch in the desired test mode */
		mipi_dsih_dphy_test_clock(instance, 1);
		/* set the desired test code in the input 8-bit bus TESTDIN[7:0] */
		mipi_dsih_dphy_test_data_in(instance, address);
		/* set TESTEN input high  */
		mipi_dsih_dphy_test_en(instance, 1);
		/* drive the TESTCLK input low; the falling edge captures the chosen test code into the transceiver */
		mipi_dsih_dphy_test_clock(instance, 0);
		/* set TESTEN input low to disable further test mode code latching  */
		mipi_dsih_dphy_test_en(instance, 0);
		/* to clear the data */
		mipi_dsih_dphy_test_data_in(instance, 0);
		/* start writing MSB first */
		for (i = data_length; i > 0 ; i--)
		{
			/* set TESTDIN[7:0] to the desired test data appropriate to the chosen test mode */
			mipi_dsih_dphy_test_data_in(instance, data[i - 1]);
			/* pulse TESTCLK high to capture this test data into the macrocell; repeat these two steps as necessary */
			mipi_dsih_dphy_test_clock(instance, 1);
			mipi_dsih_dphy_test_clock(instance, 0);
		}
#endif
	}
}



/* abstracting BSP */
/**
 * Write word to D-PHY module (encapsulating the bus interface)
 * @param instance pointer to structure which holds information about the d-phy module
 * @param reg_address offset
 * @param data 32-bit word
 */
void mipi_dsih_dphy_write_word(dphy_t * instance, uint32_t reg_address, uint32_t data)
{
	instance->core_write_function(instance->address, reg_address, data);
}
/**
 * Write bit field to D-PHY module (encapsulating the bus interface)
 * @param instance pointer to structure which holds information about the d-phy module
 * @param reg_address offset
 * @param data bits to be written to D-PHY
 * @param shift from the right hand side of the register (big endian)
 * @param width of the bit field
 */
void mipi_dsih_dphy_write_part(dphy_t * instance, uint32_t reg_address, uint32_t data, uint8_t shift, uint8_t width)
{
	uint32_t mask = (1 << width) - 1;
	uint32_t temp = mipi_dsih_dphy_read_word(instance, reg_address);
	temp &= ~(mask << shift);
	temp |= (data & mask) << shift;
	mipi_dsih_dphy_write_word(instance, reg_address, temp);
}
/**
 * Read word from D-PHY module (encapsulating the bus interface)
 * @param instance pointer to structure which holds information about the d-phy module
 * @param reg_address offset
 * @return data 32-bit word
 */
uint32_t mipi_dsih_dphy_read_word(dphy_t * instance, uint32_t reg_address)
{
	return instance->core_read_function(instance->address, reg_address);
}
/**
 * Read bit field from D-PHY module (encapsulating the bus interface)
 * @param instance pointer to structure which holds information about the d-phy module
 * @param reg_address offset
 * @param shift from the right hand side of the register (big endian)
 * @param width of the bit field
 * @return data bits to be written to D-PHY
 */
uint32_t mipi_dsih_dphy_read_part(dphy_t * instance, uint32_t reg_address, uint8_t shift, uint8_t width)
{
	return (mipi_dsih_dphy_read_word(instance, reg_address) >> shift) & ((1 << width) - 1);
}
