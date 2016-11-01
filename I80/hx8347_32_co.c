#include <common.h>
#include <malloc.h>
#include <i80.h>

struct dev_cfg reg_init[] = {
	/* GAMMA */
	{ 0x46, 0x00A4, 10 },
	{ 0x47, 0x0053, 10 },
	{ 0x48, 0x0000, 10 },
	{ 0x49, 0x0044, 10 },
	{ 0x4A, 0x0004, 10 },
	{ 0x4b, 0x0067, 10 },
	{ 0x4c, 0x0033, 10 },
	{ 0x4d, 0x0077, 10 },
	{ 0x4e, 0x0012, 10 },
	{ 0x4f, 0x004c, 10 },
	{ 0x50, 0x0046, 10 },
	{ 0x51, 0x0044, 10 },
	/* window setting 240X320 */
	{ 0x02, 0x0000, 10 },
	{ 0x03, 0x0000, 10 },
	{ 0x04, 0x0000, 10 },
	{ 0x05, 0x00ef, 10 },
	{ 0x06, 0x0000, 10 },
	{ 0x07, 0x0000, 10 },
	{ 0x08, 0x0001, 10 },
	{ 0x09, 0x003f, 10 },
	/* dispaly setting */
	{ 0x01, 0x0006, 10 },
	{ 0x16, 0x00c8, 10 },
	{ 0x38, 0x0000, 10 },
	{ 0x23, 0x0095, 10 },
	{ 0x24, 0x0095, 10 },
	{ 0x25, 0x00ff, 10 },
	{ 0x27, 0x0002, 10 },
	{ 0x28, 0x0002, 10 },
	{ 0x29, 0x0002, 10 },
	{ 0x2a, 0x0002, 10 },
	{ 0x2c, 0x0002, 10 },
	{ 0x2d, 0x0002, 10 },
	{ 0x3a, 0x0001, 10 },
	{ 0x3b, 0x0000, 10 },
	{ 0x3c, 0x00f0, 10 },
	{ 0x3d, 0x0000, 30 },
	{ 0x35, 0x0038, 10 },
	{ 0x36, 0x0078, 10 },
	{ 0x3e, 0x0038, 10 },
	{ 0x40, 0x000f, 10 },
	{ 0x41, 0x00f0, 10 },
	/* Power supply setting */
	{ 0x19, 0x0049, 10 },
	{ 0x93, 0x000f, 20 },
	{ 0x20, 0x0040, 10 },
	{ 0x1d, 0x0007, 10 },
	{ 0x1e, 0x0000, 10 },
	{ 0x1f, 0x0004, 10 },
	/* VCOM SETTING for CMO2.4 panel */
	{ 0x44, 0x004D, 10 },
	{ 0x45, 0x0011, 20 },
	{ 0x1c, 0x0004, 30 },
	{ 0x1b, 0x0018, 50 },
	{ 0x1b, 0x0010, 50 },
	{ 0x43, 0x0080, 120 },
	/* display on setting */
	{ 0x90, 0x007f, 10 },	
	{ 0x26, 0x0004, 50 },
	{ 0x26, 0x0024, 10 },
	{ 0x26, 0x002c, 50 },
	{ 0x26, 0x003c, 10 },
	/* internal register setting */
	{ 0x57, 0x0002, 10 },
	{ 0x95, 0x0001, 10 },
	{ 0x57, 0x0000, 10 },
};

static void hx8347_write(uint32_t reg, int data)
{
	i80_mannual_init();	
	i80_mannual_ctrl(I80_MAN_RS, I80_ACTIVE_LOW);	
	i80_mannual_ctrl(I80_MAN_CS0, I80_ACTIVE_LOW);
	i80_mannual_write_once(reg);
	i80_mannual_ctrl(I80_MAN_RS, I80_ACTIVE_HIGH);	
	i80_mannual_write_once(data);
	i80_mannual_ctrl(I80_MAN_CS0, I80_ACTIVE_HIGH);
	i80_mannual_deinit();	
}

#if 0
static int hx8347_read(uint32_t reg)
{
	uint32_t value;
	//uint32_t high_byte, low_byte;

	i80_mannual_init();
	i80_mannual_ctrl(I80_MAN_RS, I80_ACTIVE_LOW);
	i80_mannual_ctrl(I80_MAN_CS0, I80_ACTIVE_LOW);
#if 0
	high_byte = (reg & 0xff00) << 2;
	low_byte = (reg & 0x00ff) << 1;
	value = high_byte | low_byte;
#endif
	i80_mannual_write_once(value);
	i80_mannual_ctrl(I80_MAN_RS, I80_ACTIVE_HIGH);
	i80_mannual_ctrl(I80_MAN_RS, I80_ACTIVE_LOW);
	value = i80_mannual_read_once();
#if 0
	high_byte = (value >> 2) & 0xff00;	
	low_byte = (value >> 1) & 0x00ff;
	value = high_byte | low_byte;
#endif
	i80_mannual_ctrl(I80_MAN_RS, I80_ACTIVE_HIGH);
	i80_mannual_deinit();
	return value;
}
#endif

static int hx8347_config(uint8_t *mem)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(reg_init); i++) {
		hx8347_write(reg_init[i].addr, reg_init[i].data);
		udelay(reg_init[i].delay * 1000);
		//printf("0x%x: 0x%x\n", hx8347_read(reg_init[i].addr));
	}
	
	if (mem)	
		hx8347_mannual_write((uint32_t *)mem, 240*320, I80IF_BUS16_00_11_x_x);
	return 0;
}

static int hx8347_open(void)
{
	/* reset i80 lcd */
	pads_set_mode(40, PADS_MODE_OUT, 0, CPU_TYPE_AP);
	udelay(3);
	pads_set_mode(40, PADS_MODE_OUT, 1, CPU_TYPE_AP);
	udelay(10);
	pads_set_mode(40, PADS_MODE_OUT, 0, CPU_TYPE_AP);
	udelay(20);
	pads_set_mode(40, PADS_MODE_OUT, 1, CPU_TYPE_AP);
	udelay(120000);
	return 0;
}

static int hx8347_close(void)
{
	return 0;
}

int hx8347_mannual_write(uint32_t *mem, int length, int mode)
{
	uint32_t tmp, i;
	uint32_t red, green, blue, blue2;

	i80_mannual_init();
	i80_mannual_ctrl(I80_MAN_RS, I80_ACTIVE_LOW);
	i80_mannual_ctrl(I80_MAN_CS0, I80_ACTIVE_LOW);
	switch (mode) {
		case I80IF_BUS16_00_11_x_x:
			i80_mannual_write_once(0x0022);
			i80_mannual_ctrl(I80_MAN_RS, I80_ACTIVE_HIGH);
			for (i = 0; i < length; i++) {
				red = (mem[i] & 0xff0000) >> (16 + 3);
				green = (mem[i] & 0x00ff00) >> (8 + 2);
				blue = (mem[i] & 0x0000ff) >> (0 + 3);
				tmp = (red << 11) | (green << 5) | (blue);
				i80_mannual_write_once(tmp);
			}
			break;
		case I80IF_BUS16_00_10_0_1:
			i80_mannual_write_once(0x0022);
			i80_mannual_ctrl(I80_MAN_RS, I80_ACTIVE_HIGH);
			for (i = 0; i < length; i++) {
				red = (mem[i] & 0xff0000) >> (16 + 2); 
				green = (mem[i] & 0x00ff00) >> (8 + 2);
				blue = (mem[i] & 0x0000ff) >> (0 + 4); 
				blue2 = (mem[i] & 0x00000f) >> (0 + 2);
				tmp = (red << 11) | (green << 5) | (blue);
				i80_mannual_write_once(tmp);
				i80_mannual_write_once(blue2);
			}
			break;
		default:
			printf("Now, not support this mode: %d\n", mode);	
			break;		
	}
	i80_mannual_ctrl(I80_MAN_CS0, I80_ACTIVE_HIGH);
	i80_mannual_deinit();
	return 0;
}

int hx8347_init(void)
{
	struct i80_dev *hx8347;

	hx8347 = (struct i80_dev *)malloc(sizeof(struct i80_dev));
	if (!hx8347)
		return -1;
		
	hx8347->name = "hx8347";
	hx8347->config = hx8347_config;
	hx8347->open = hx8347_open;
	hx8347->close = hx8347_close;
	hx8347->write = hx8347_mannual_write;

	return i80_register(hx8347);
}
