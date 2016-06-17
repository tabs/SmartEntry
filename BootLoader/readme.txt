


此BOOTLOADER实现程序的远程更新

其中宏定义SysCordAppUpdataFlag 0x7E00  这是升级标志两个字节的储存地址
UpdataFlag0 0x5A/*是否升级判断标志 0x5a 0xa5*/
UpdataFlag1 0xA5
SysCordStartAddr 0x1000 /*程序应用区的起始地址*/
SysCordEndAddr 0x7200 /*程序应用区的结束地址*/
SysCordAppSaveStartTempAddr 0x8000 /*程序应用区暂存的起始地址*/