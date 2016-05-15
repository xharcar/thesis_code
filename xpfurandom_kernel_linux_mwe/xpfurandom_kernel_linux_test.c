// xpfurandom_kernel_linux_test.c
// testing xpfurandom kernel fuctionality

#include "xpfurandom_kernel.h"

static int __init xpfurandom_kernel_init(void){
	void* data;
	int c;
	pr_notice("XPFUrandom: init started, variables declared.\n");
	c = xpfurandom_kernel_prep(&data,256,&data);
	if(c!=0){
		pr_err("XPFUrandom: prep failed\n");
		return -1;
	}
	c = xpfurandom_kernel_get_random_data(&data,256,&data);
	if(c!=0){
		pr_err("XPFUrandom: get_random_data failed\n");
		return -2;
	}
	c = xpfurandom_kernel_cleanup(&data,256,&data);
	if(c!=0){
		pr_err("XPFUrandom: cleanup failed\n");
		return -3;
	}
	pr_notice("XPFUrandom: init successful.\n");
	return 0;
}

static void __exit xpfurandom_kernel_exit(void)
{
	pr_notice("XPFUrandom: Module unloaded.\n");
}


module_init(xpfurandom_kernel_init);
module_exit(xpfurandom_kernel_exit);

MODULE_AUTHOR("Matej Harcar, 422714@mail.muni.cz");
MODULE_DESCRIPTION("XPFUrandom Kernel test module");
MODULE_LICENSE("GPL");
