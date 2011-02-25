#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>

int external_memory_test(void)
{
	int return_value = 0;
	char *src = (void *)0;
	char *dest = (void *)0;
	off_t fd_offset;
	int fd;

	if ( (fd = sys_open((const char __user *) "/sdcard/SDTest.txt", O_CREAT | O_RDWR, 0) ) < 0 )
	{
		printk(KERN_ERR "[ATCMD_EMT] Can not access SD card\n");
		goto file_fail;
	}

	if ( (src = kmalloc(10, GFP_KERNEL)) )
	{
		sprintf(src,"TEST");
		if((sys_write(fd, (const char __user *) src, 5)) < 0)
		{
			printk(KERN_ERR "[ATCMD_EMT] Can not write SD card \n");
			goto file_fail;
		}
		fd_offset = sys_lseek(fd, 0, 0);
	}
	if ( (dest = kmalloc(10, GFP_KERNEL)) )
	{
		if((sys_read(fd, (char __user *) dest, 5)) < 0)
		{
			printk(KERN_ERR "[ATCMD_EMT]Can not read SD card \n");
			goto file_fail;
		}
		if ((memcmp(src, dest, 4)) == 0)
			return_value = 1;
		else
			return_value = 0;
	}

	sys_close(fd);
	sys_unlink((const char __user *)"/sdcard/SDTest.txt");
	kfree(src);
	kfree(dest);
	return return_value;

file_fail:
	sys_close(fd);
	sys_unlink((const char __user *)"/sdcard/SDTest.txt");
	return return_value;
}
