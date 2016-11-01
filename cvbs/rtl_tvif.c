#include <common.h>
#include <malloc.h>
#include <cvbs.h>


static int rtl_cvbs_cfg(int flags)
{
	return 0;
}

static int rtl_cvbs_open(void)
{
	return 0;
}

static int rtl_cvbs_close(void)
{
	return 0;
}

int rtl_cvbs_init(void)
{
	struct cvbs_dev *rtl;

	rtl = (struct cvbs_dev *)malloc(sizeof(struct cvbs_dev));
	if (!rtl)
		return -1;

	rtl->name = "rtl_cvbs";
	rtl->config = rtl_cvbs_cfg;
	rtl->open = rtl_cvbs_open;
	rtl->close = rtl_cvbs_close;

	return cvbs_register(rtl);
}
