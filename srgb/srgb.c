#include <common.h>
#include <linux/list.h>
#include <srgb.h>

LIST_HEAD(srgb_list);
static struct srgb_dev *dev = NULL;

int srgb_open(char *name)
{
	list_for_each_entry(dev, &srgb_list, link) {
		if(strcmp(dev->name, name) == 0)
			return dev->open();
	}
	return -1;
}

int srgb_close(void)
{
	if (!dev)
		return -1;
	
	return dev->close();
}	

int srgb_dev_cfg(void)
{
	if (!dev)
		return -1;

	return dev->config();
}

int srgb_register(struct srgb_dev *udev)
{
	if (!udev)
		return -1;

	list_for_each_entry(dev, &srgb_list, link)
		if(strcmp(dev->name, udev->name) == 0)
			return 0;

	list_add_tail(&(udev->link), &srgb_list);
	return 0;
}
