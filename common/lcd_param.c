static struct lcd_param panel[] = {
	[0] = {
		.name = "KR070LB0S_1024_600",
		.mPixelClock = 51200,
		.mHActive = 1024,
		.mVActive = 600,
		.mHBackPorch = 160,
		.mVBackPorch = 23,
		.mHFrontPorch = 80,
		.mVFrontPorch = 6,
		.mHSyncPulseWidth = 80,
		.mVSyncPulseWidth = 6,
		.mHSyncPolarity = 0,
		.mVSyncPolarity = 0,
		.VclkPolarity = 0,
		.VDPolarity = 0,
		.VdenPolarity = 0,
		.Vpwren = 0,
		.Pwren = 1,
		.rgb_seq = RGB_ORDER,
		.rgb_bpp = RGB888,
		.display_type = TFT_LCD,
	},
	[1]	= {
		.name = "B079XAN01_768_1024",
		.mHActive = 768,
		.mVActive = 1024,
		.mHBackPorch = 56,
		.mVBackPorch = 48,
		.mHFrontPorch = 60,
		.mVFrontPorch = 36,    
		.mHSyncPulseWidth = 64,
		.mVSyncPulseWidth = 32,
		.mHSyncPolarity = 1,   
		.mVSyncPolarity = 1,   
		.mPixelClock = 60000,  
		.VclkPolarity = 0,   
		.VDPolarity = 0,     
		.VdenPolarity = 0,   
		.Vpwren = 0,         
		.Pwren = 1,          
		.rgb_seq = RGB_ORDER,
		.rgb_bpp = RGB888,
		.display_type = TFT_LCD,	
	},
	[2] = {
		.name = "HDMI 720P",
		.mHActive = 1280,
		.mVActive = 720,
		.mHBackPorch = 220,
		.mVBackPorch = 20,
		.mHFrontPorch = 110,
		.mVFrontPorch = 5,    
		.mHSyncPulseWidth = 40,
		.mVSyncPulseWidth = 5,
		.mHSyncPolarity = 1,   
		.mVSyncPolarity = 1,   
		.mPixelClock = 74250,  
		.VclkPolarity = 0,   
		.VDPolarity = 0,     
		.VdenPolarity = 0,   
		.Vpwren = 0,         
		.Pwren = 1,          
		.rgb_seq = RGB_ORDER,
		.rgb_bpp = RGB888,
		.display_type = TFT_LCD,
	},
	[3] = {
		.name = "HDMI 480P",
		.mHActive = 640,
		.mVActive = 480,
		.mHBackPorch = 48,
		.mVBackPorch = 33,
		.mHFrontPorch = 16,
		.mVFrontPorch = 10,    
		.mHSyncPulseWidth = 96,
		.mVSyncPulseWidth = 2,
		.mHSyncPolarity = 0,   
		.mVSyncPolarity = 0,   
		.mPixelClock = 600000,  
		.VclkPolarity = 1,   
		.VDPolarity = 0,     
		.VdenPolarity = 0,   
		.Vpwren = 0,         
		.Pwren = 1,          
		.rgb_seq = RGB_ORDER,
		.rgb_bpp = RGB888,
		.display_type = TFT_LCD,
	},
	[4] = {
		.name = "KR070PB2S_800_480",
		.mPixelClock = 33260,
		.mHActive = 800,
		.mVActive = 480,
		.mHBackPorch = 210,
		.mVBackPorch = 22,
		.mHFrontPorch = 46,
		.mVFrontPorch = 23,
		.mHSyncPulseWidth = 30,
		.mVSyncPulseWidth = 13,
		.mHSyncPolarity = 0,
		.mVSyncPolarity = 0,
		.VclkPolarity = 0,
		.VDPolarity = 0,
		.VdenPolarity = 0,
		.Vpwren = 0,
		.Pwren = 1,
		.rgb_seq = RGB_ORDER,
		.rgb_bpp = RGB888,
		.display_type = TFT_LCD,
	},
	[5] = {
		.name = "HDMI 1080P",
		.mHActive = 1920,
		.mVActive = 1080,
		.mHBackPorch = 148,
		.mVBackPorch = 36,
		.mHFrontPorch = 88,
		.mVFrontPorch = 4,    
		.mHSyncPulseWidth = 44,
		.mVSyncPulseWidth = 5,
		.mHSyncPolarity = 0,   
		.mVSyncPolarity = 0,   
		.mPixelClock = 148500,  
		.VclkPolarity = 1,   
		.VDPolarity = 0,     
		.VdenPolarity = 0,   
		.Vpwren = 0,         
		.Pwren = 1,          
		.rgb_seq = RGB_ORDER,
		.rgb_bpp = RGB888,
		.display_type = TFT_LCD,
	},
	[6] = {
		.name = "I80 320",
		.mPixelClock = 3392,
		.mHActive = 240,
		.mVActive = 320,
		.mHBackPorch = 48,
		.mVBackPorch = 100,
		.mHFrontPorch = 16,
		.mVFrontPorch = 2,
		.mHSyncPulseWidth = 96,
		.mVSyncPulseWidth = 2,
		.mHSyncPolarity = 0,
		.mVSyncPolarity = 0,
		.VclkPolarity = 0,
		.VDPolarity = 0,
		.VdenPolarity = 0,
		.Vpwren = 0,
		.Pwren = 0,
		.rgb_seq = RGB_ORDER,
		.rgb_bpp = RGB888,
		.display_type = TFT_LCD,
	},
	[7] = {
		.name = "YL_640_480",
		.mHActive = 640,
		.mVActive = 480,
		.mHBackPorch = 48,
		.mVBackPorch = 33,
		.mHFrontPorch = 16,
		.mVFrontPorch = 10,    
		.mHSyncPulseWidth = 96,
		.mVSyncPulseWidth = 2,
		.mHSyncPolarity = 0,   
		.mVSyncPolarity = 0,   
		.mPixelClock = 600000,  
		.VclkPolarity = 1,   
		.VDPolarity = 0,     
		.VdenPolarity = 0,   
		.Vpwren = 0,         
		.Pwren = 1,          
		.rgb_seq = RGB_ORDER,
		.rgb_bpp = RGB888,
		.display_type = TFT_LCD,
	},
	[8] = {
		.name = "YL_800_600",
		.mPixelClock = 41376,
		.mHActive = 800,
		.mVActive = 600,
		.mHBackPorch = 210,
		.mVBackPorch = 23,
		.mHFrontPorch = 46,
		.mVFrontPorch = 6,
		.mHSyncPulseWidth = 30,
		.mVSyncPulseWidth = 6,
		.mHSyncPolarity = 0,
		.mVSyncPolarity = 0,
		.VclkPolarity = 0,
		.VDPolarity = 0,
		.VdenPolarity = 0,
		.Vpwren = 0,
		.Pwren = 1,
		.rgb_seq = RGB_ORDER,
		.rgb_bpp = RGB888,
		.display_type = TFT_LCD,
	},
	[9] = {
		.name = "YL_320_240",
		.mPixelClock = 5334,
		.mHActive = 320,
		.mVActive = 240,
		.mHBackPorch = 10,
		.mVBackPorch = 10,
		.mHFrontPorch = 10,
		.mVFrontPorch = 2,
		.mHSyncPulseWidth = 10,
		.mVSyncPulseWidth = 2,
		.mHSyncPolarity = 0,
		.mVSyncPolarity = 0,
		.VclkPolarity = 0,
		.VDPolarity = 0,
		.VdenPolarity = 0,
		.Vpwren = 0,
		.Pwren = 1,
		.rgb_seq = RGB_ORDER,
		.rgb_bpp = RGB888,
		.display_type = TFT_LCD,
	},
	[10] = {
		.name = "HD_1920_1080",
		.mHActive = 1920,
		.mVActive = 1080,
		.mHBackPorch = 1,
		.mVBackPorch = 1,
		.mHFrontPorch = 1,
		.mVFrontPorch = 1,    
		.mHSyncPulseWidth = 1,
		.mVSyncPulseWidth = 1,
		.mHSyncPolarity = 0,   
		.mVSyncPolarity = 0,   
		.mPixelClock = 125000,  
		.VclkPolarity = 1,   
		.VDPolarity = 0,     
		.VdenPolarity = 0,   
		.Vpwren = 0,         
		.Pwren = 1,          
		.rgb_seq = RGB_ORDER,
		.rgb_bpp = RGB888,
		.display_type = TFT_LCD,
	},
	[11] = {
                .name = "360_240",
                .mPixelClock = 2402,
                .mHActive = 360,
                .mVActive = 240,
                .mHBackPorch = 111,
                .mVBackPorch = 13,
                .mHFrontPorch = 3,
                .mVFrontPorch = 5,
                .mHSyncPulseWidth = 126,
                .mVSyncPulseWidth = 8,
                .mHSyncPolarity = 1,
                .mVSyncPolarity = 1,
                .VclkPolarity = 1,
                .VDPolarity = 0,
                .VdenPolarity = 0,
                .Vpwren = 0,
                .Pwren = 0,
                .rgb_seq = RGB_ORDER,
                .rgb_bpp = RGB888,
                .display_type = TFT_LCD,
        },
	[12] = {
                .name = "320_240",
                .mPixelClock = 2402,
                .mHActive = 320,
                .mVActive = 240,
                .mHBackPorch = 70,
                .mVBackPorch = 21,
                .mHFrontPorch = 686,
                .mVFrontPorch = 1,
                .mHSyncPulseWidth = 1,
                .mVSyncPulseWidth = 1,
                .mHSyncPolarity = 1,
                .mVSyncPolarity = 1,
                .VclkPolarity = 1,
                .VDPolarity = 0,
                .VdenPolarity = 0,
                .Vpwren = 0,
                .Pwren = 0,
                .rgb_seq = RGB_ORDER,
                .rgb_bpp = RGB888,
                .display_type = TFT_LCD,
        },
};
