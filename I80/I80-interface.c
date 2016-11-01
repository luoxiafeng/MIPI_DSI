#include <common.h>
#include <linux/list.h>
#include <i80.h>

LIST_HEAD(i80_list);
static struct i80_dev *dev = NULL;

int i80_open(char *name)
{
	list_for_each_entry(dev, &i80_list, link) {
		if(strcmp(dev->name, name) == 0)
			return dev->open();
	}
	return -1;
}

int i80_close(void)
{
	if (!dev)
		return -1;
	
	return dev->close();
}

int i80_mannual_write(uint32_t *mem, int length, int mode)
{
	if (!dev)
		return -1;
		
	return dev->write(mem, length, mode);
}	

int i80_dev_cfg(uint8_t *mem, int format)
{
	int ret;

	if (!dev)
		return -1;
	
	i80_default_cfg(format);

	i80_enable(1);
	ret = dev->config(mem);
	i80_enable(0);

	return ret;
}

int i80_register(struct i80_dev *udev)
{
	if (!udev)
		return -1;

	list_for_each_entry(dev, &i80_list, link)
		if(strcmp(dev->name, udev->name) == 0)
			return 0;

	list_add_tail(&(udev->link), &i80_list);
	return 0;
}
