


2016.12.24: 油门舵机工作正常
2016.12.26: 开始emWin
2016.12.27: 参考emwin v1.2.0 STM324xG-EVAL
2016.12.29: 	
	DEMO显示正常，修改了LCD驱动0x3600寄存器，触屏驱动坐标
	中断文件中，声明extern volatile GUI_TIMER_TIME OS_TimeMS;  并在TICK服务程序中使OS_TimeMS++
2017.1.9
	GUI可正常显示油门舵机指针，2通道显示错乱是因为_DrawCh2坐标错误
2017.1.10
	数字显示由于在数字更新是未清除原来的字符，导致显示重叠，在写数字前GUI_ClearRect()显示为背景颜色,会带来闪烁
    使用GUI_MEMDEV_CreateAuto（）创建存储设备解决闪烁问题，不需要GUI_ClearRect()
	
	