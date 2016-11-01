
ARCHIVE := drv_hdmi.a

SRCS := \
		$(wildcard *.c)							\
		$(wildcard synopsys_hdmi/api/*.c)		\
		$(wildcard synopsys_hdmi/bsp/*.c)		\
		$(wildcard synopsys_hdmi/core/*.c)	\
		$(wildcard synopsys_hdmi/edid/*.c)	\
		$(wildcard synopsys_hdmi/hdcp/*.c)	\
		$(wildcard synopsys_hdmi/phy/*.c)		\
		$(wildcard synopsys_hdmi/util/*.c)	\

CUSTOM_CFLAGS := -Wall -Wstrict-prototypes -Wno-unused-but-set-variable -D__KERNEL__  -O1 \
		  -I$(CURDIR)/synopsys_hdmi/core/ -I$(CURDIR)/synopsys_hdmi/bsp/	\
		  -I$(CURDIR)/synopsys_hdmi/util/ -I$(CURDIR)/synopsys_hdmi/edid/	\
		  -I$(CURDIR)/synopsys_hdmi/hdcp/ -I$(CURDIR)/synopsys_hdmi/phy/	\
		  -I$(CURDIR)/synopsys_hdmi/api/

include $(BUILD_LIB)
