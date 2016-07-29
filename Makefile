
ARCHIVE := drv_mipi.a

SRCS := mipi.c	\
		synopsys_mipi_dsi_1.00a/mipi_dsih_api.c \
		synopsys_mipi_dsi_1.00a/mipi_dsih_hal.c \
		synopsys_mipi_dsi_1.00a/mipi_dsih_dphy.c \

include $(BUILD_LIB)

