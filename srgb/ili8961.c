#include <common.h> 
#include <malloc.h> 
#include <flash.h>
#include <ssp.h>
#include <asm/imapx-regs.h>
#include <srgb.h>

static int ili8961_cfg_rgbdummy(void)
{
	uint8_t tx[18] = {
		0x05, 0x5f, 0x05, 0x1f, 0x05, 0x5f, 0x2B, 0x01, 0x00, 0x07, 0x01, 0x90, 0x04, 0x2B,
		0x16, 0x04, 0x0D, 0x40
	};
	spi_protl_set spi_trans;
	int i;
	ssp_manager_init();
	ssp_manager_set_wire(15);
	//flash_vs_init();
	flash_vs_reset();

	for (i = 0; i < 9; i++) {
		spi_transfer_init(&spi_trans);
		spi_trans.commands[0] = tx[2 * i];
		spi_trans.commands[1] = tx[2 * i + 1];
		spi_trans.cmd_count = 2;
		spi_trans.cmd_type = CMD_ONLY;
		spi_trans.module_lines = 15;
		spi_read_write_data(&spi_trans);
	}
	printf("ili8961_cfg_rgbdummy\n");
	return 0;
}

static int ili8961_cfg_rgb(void)
{
	/*uint8_t tx[18] = {
		0x05, 0x5f, 0x05, 0x1f, 0x05, 0x5f, 0x2B, 0x01, 0x00, 0x07, 0x01, 0x90, 0x04, 0x0B,
		0x16, 0x04, 0x0D, 0x40
	};*/
	uint8_t tx[20] = {
		0x05, 0x5f, 0x05, 0x1f, 0x05, 0x5f, 0x2B, 0x01, 0x00, 0x07, 0x01, 0x90, 0x04, 0x0B,
		0x16, 0x04, 0x0D, 0x40, 0x13, 0x01
	};
	spi_protl_set spi_trans;
	spi_manager_glob  spi_mgr;
	int i;

	spi_mgr.g_wires  = 15;
	ssp_manager_init();
	ssp_manager_set_wire(15);                        

	//flash_vs_init();
	flash_vs_reset();

	for (i = 0; i < 10; i++) {
	char abc = 0;
		spi_transfer_init(&spi_trans);
		spi_trans.commands[0] = tx[2 * i];
		spi_trans.commands[1] = tx[2 * i + 1];
		spi_trans.cmd_count = 2;
		spi_trans.cmd_type = CMD_ONLY;
		spi_trans.module_lines = 15;
		spi_read_write_data(&spi_trans);
			
		spi_trans.commands[0] = tx[2 * i]|(1<<6);
		spi_trans.cmd_type = CMD_READ_DATA;
		spi_trans.cmd_count = 1;
		spi_trans.read_count = 1;
		spi_trans.gen_read_clk_count = spi_trans.read_count;
		spi_trans.read_buf = (void*)&abc;
		spi_read_write_data(&spi_trans);
		printf("spi read 0x%02x,0x%02x\n",tx[2*i+1],abc);
		
	}
	printf("ili8961_cfg_rgb\n");
	return 0;
}
static int ili8961_open(void)
{
	printf("ili8961_open\n");
	return 0;
}

static int ili8961_close(void)
{
	printf("ili8961_close\n");
	return 0;
}

int ili8961_init(int flag)
{
	struct srgb_dev *srgb;

	srgb = (struct srgb_dev *)malloc(sizeof(struct srgb_dev));
	if (!srgb)
		return -1;

	srgb->name = "ili8961";
	if(flag==1)
		srgb->config = ili8961_cfg_rgbdummy;
	else if(flag==2)
		srgb->config = ili8961_cfg_rgbdummy;
	else
		srgb->config = ili8961_cfg_rgb;
	srgb->open = ili8961_open;
	srgb->close = ili8961_close;

	return srgb_register(srgb);
}
