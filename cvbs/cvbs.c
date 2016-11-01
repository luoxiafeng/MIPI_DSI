#include <common.h>
#include <linux/list.h>
#include <cvbs.h>

LIST_HEAD(cvbs_list);
static struct cvbs_dev *dev = NULL;

int cvbs_open(char *name)
{
	list_for_each_entry(dev, &cvbs_list, link) {
		if(strcmp(dev->name, name) == 0)
			return dev->open();
	}
	return -1;
}

int cvbs_close(void)
{
	if (!dev)
		return -1;
	
	return dev->close();
}	

int cvbs_dev_cfg(int flags)
{
	if (!dev)
		return -1;

	return dev->config(flags);
}

int cvbs_register(struct cvbs_dev *udev)
{
	if (!udev)
		return -1;

	list_for_each_entry(dev, &cvbs_list, link)
		if(strcmp(dev->name, udev->name) == 0)
			return 0;

	list_add_tail(&(udev->link), &cvbs_list);
	return 0;
}
