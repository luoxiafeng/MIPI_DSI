#include <common.h>
#include <malloc.h>
#include <cvbs.h>
#include <iic.h>

static struct cvbs_i2c_cfg cfg = {
	.name = "gm7122",
	.i2c_num = 1,
	.i2c_addr = 0x44,
	.cvbs_reset = 75,
};

struct cvbs_i2c common1[] = {
	{0x28,0x21},
	{0x29,0x1D},
	{0x2A,0x00},
	{0x2B,0x00},
	{0x2C,0x00},
	{0x2D,0x00},
	{0x2E,0x00},
	{0x2F,0x00},
	{0x30,0x00},
	{0x31,0x00},
	{0x32,0x00},
	{0x33,0x00},
	{0x34,0x00},
	{0x35,0x00},
	{0x36,0x00},
	{0x37,0x00},
	{0x38,0x00},
	{0x39,0x00},
	{0x3A,0x13},
	{0x5A,0x00},
	{0x5B,0x6D},
	{0x5C,0x9F},
	{0x5D,0x0e},
	{0x5E,0x1C},
	{0x5F,0x35},
	{0x60,0x00},
};

struct cvbs_i2c common2[] = {
	{0x62,0x3B},
	{0x6B,0x00},
	{0x6C,0xff},
	{0x6D,0xff},
	{0x75,0xff},
};

struct cvbs_i2c gm7122_pal[] = {
	{0x61,0x06},
	{0x63,0xCB},
	{0x64,0x8A},
	{0x65,0x09},
	{0x66,0x2A},
};

struct cvbs_i2c gm7122_ntsc[] = {
	{0x61,0x01},
	{0x63,0x1F},
	{0x64,0x7C},
	{0x65,0xF0},
	{0x66,0x21},
};

struct cvbs_i2c valid_regs[] = {
	{ 0x28, 0 },
	{ 0x29, 0 },
	{ 0x2F, 0 },
	{ 0x3A, 0 },
	{ 0x5A, 0 },
	{ 0x5B, 0 },
	{ 0x5C, 0 },
	{ 0x5D, 0 },
	{ 0x5E, 0 },
	{ 0x61, 0 },
	{ 0x62, 0 },
	{ 0x63, 0 },
	{ 0x64, 0 },
	{ 0x65, 0 },
	{ 0x66, 0 },
	{ 0x6B, 0 },
	{ 0x6C, 0 },
	{ 0x6D, 0 },
	{ 0x75, 0 },
};

static void gm7122_i2c_read(struct cvbs_i2c *i2c, int num)
{
	struct i2c_msg msg[2] = { 
		{ cfg.i2c_addr, 0, 1, NULL },
		{ cfg.i2c_addr, I2C_M_RD, 1, NULL },
	};
	int i;

	for (i = 0; i < num; i++) {
		msg[0].buf = &(i2c[i].addr);
		msg[1].buf = &(i2c[i].val);

		if (i2c_transfer(cfg.i2c_num, msg, 2) != 2)
			cvbs_err("gm7122 read err.\n");
	}
}

static void gm7122_i2c_write(struct cvbs_i2c *i2c, int num)
{
	uint8_t tx_buf[2];
	int i;

	for (i = 0; i < num; i++) {
		tx_buf[0] = i2c[i].addr;
		tx_buf[1] = i2c[i].val;

		if (i2c_write(cfg.i2c_num, cfg.i2c_addr, tx_buf, 2) != 1)
			cvbs_err("gm7122 write err\n");
	}
}
#if 0
static void gm7122_dump_reg(void)
{
	int i;

	gm7122_i2c_read(valid_regs, ARRAY_SIZE(valid_regs));
	for (i = 0; i < ARRAY_SIZE(valid_regs); i++)
		printf("reg: [0x%x, 0x%x]\n", valid_regs[i].addr, valid_regs[i].val);
}
#endif

static int gm7122_check_cfg(int flags)
{
	struct cvbs_i2c i2c[5];
	int ret, i;
	
	for (i = 0; i < 5; i++)
		i2c[i].addr = gm7122_pal[i].addr;

	gm7122_i2c_read(i2c, 5);
	if (flags == f_PAL)
		ret = strncmp((char *)i2c, (char *)gm7122_pal, sizeof(struct cvbs_i2c) * 5);
	else
		ret = strncmp((char *)i2c, (char *)gm7122_ntsc, sizeof(struct cvbs_i2c) * 5);
#if 0
	for (i = 0; i < 5; i++)
		printf("PAL: [0x%x, 0x%x]\n", i2c[i].addr, i2c[i].val);
#endif
	//gm7122_dump_reg();
	return ret;
}

static int gm7122_config(int flags)
{
	/* GM7122 digital core work depend on 27MHz clock through LLC pin. 
	 So, TVIF CLK output must be avaliable before I2C communication. */
	udelay(50000);

	gm7122_i2c_write(common1, ARRAY_SIZE(common1));
	if (flags == f_PAL) {
		gm7122_i2c_write(gm7122_pal, ARRAY_SIZE(gm7122_pal));
	} else {
		gm7122_i2c_write(gm7122_ntsc, ARRAY_SIZE(gm7122_ntsc));
	}
	gm7122_i2c_write(common2, ARRAY_SIZE(common2));
	
	udelay(50000);
	return gm7122_check_cfg(flags);
}

static int gm7122_open(void)
{
	cvbs_dbg("%s\n", __func__);
	/* init i2c */
	i2c_init(cfg.i2c_num, 100000);
	/* reset cvbs */
	if (cfg.cvbs_reset) {
		pads_set_mode(cfg.cvbs_reset, PADS_MODE_OUT, 1, CPU_TYPE_AP);
		udelay(250000);
		pads_set_value(cfg.cvbs_reset, 0, CPU_TYPE_AP);
		udelay(250000);
		pads_set_value(cfg.cvbs_reset, 1, CPU_TYPE_AP);
	}
	return 0;
}

static int gm7122_close(void)
{
	cvbs_dbg("%s\n", __func__);
	if (cfg.cvbs_reset) {
		pads_set_value(cfg.cvbs_reset, 0, CPU_TYPE_AP);
		udelay(500000);
	}
	return 0;
}

int gm7122_init(void)
{
	struct cvbs_dev *gm7122;	

	gm7122 = (struct cvbs_dev *)malloc(sizeof(struct cvbs_dev));
	if (!gm7122)
		return -1;

	gm7122->name = "gm7122";
	gm7122->config = gm7122_config;
	gm7122->open = gm7122_open;
	gm7122->close = gm7122_close;

	return cvbs_register(gm7122);
}
