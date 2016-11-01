#include <common.h>
#include <malloc.h>
#include <i80.h>

static int rtl_i80_config(uint8_t *mem)
{
	return 0;
}

static int rtl_i80_open(void)
{
	return 0;
}

static int rtl_i80_close(void)
{
	return 0;
}

int rtl_i80_mannual_write(uint32_t *mem, int length, int mode)
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

int rtl_i80_init(void)
{
	struct i80_dev *rtl_i80;

	rtl_i80 = (struct i80_dev *)malloc(sizeof(struct i80_dev));
	if (!rtl_i80)
		return -1;
		
	rtl_i80->name = "rtl_i80";
	rtl_i80->config = rtl_i80_config;
	rtl_i80->open = rtl_i80_open;
	rtl_i80->close = rtl_i80_close;
	rtl_i80->write = rtl_i80_mannual_write;

	return i80_register(rtl_i80);
}
