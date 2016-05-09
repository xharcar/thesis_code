// xpfurandom_kernel_linux_test.c
// testing xpfurandom kernel fuctionality

#include "xpfurandom_kernel.h"

static int xpfurandom_kernel_prep(void** buffer, int size, void** win_alg_handle) {
	pr_alert("kernel_prep launched\n");
	*buffer = kmalloc(size, GFP_KERNEL);
	if (*buffer == NULL) return 1;
	printk(KERN_ALERT "kernel memory allocated\n");
	return 0;
}
static int xpfurandom_kernel_get_random_data(void** buffer, int size, void** win_alg_handle) {
	int i;
	int8_t* x = (int8_t*) *buffer;
	pr_alert("XPFUrandom: kernel_get_random_data running\n");
	get_random_bytes(*buffer, size); 
	pr_alert("XPFUrandom: data follows\n");
	for(i = 0;i < size;i++) pr_alert("%x ",x[i]);
	// has void return, hopefully does not fail
	pr_alert("XPFUrandom: random data acquired\n");
	return 0;
}
static int xpfurandom_kernel_cleanup(void** buffer, int size, void** win_alg_handle) {
	char* x = (char*) *buffer;
	pr_alert("kernel_cleanup\n");
	memset(x,0,size);
	if(x[1] != 0) {
		return -1;
	}
	kfree(*buffer);
	pr_alert("cleanup done\n");
	return 0;
}

static int __init xpfurandom_kernel_init(void){
	void* data;
	int c;
	pr_alert("XPFUrandom: variables declared.\n");
	c = xpfurandom_kernel_prep(&data,256,&data);
	if(c!=0){
		pr_alert("XPFUrandom: prep failed\n");
		return -1;
	}
	c = xpfurandom_kernel_get_random_data(&data,256,&data);
	if(c!=0){
		pr_alert("XPFUrandom: get_random_data failed\n");
		return -2;
	}
	c = xpfurandom_kernel_cleanup(&data,256,&data);
	if(c!=0){
		pr_alert("XPFUrandom: memory overwrite failed\n");
		return -3;
	}
	pr_alert("XPFUrandom: init OK\n");
	return 0;
}

static void __exit xpfurandom_kernel_exit(void)
{
	pr_alert("XPFUrandom Kernel Module unloaded.\n");
}


module_init(xpfurandom_kernel_init);
module_exit(xpfurandom_kernel_exit);

MODULE_AUTHOR("Matej Harcar, 422714@mail.muni.cz");
MODULE_DESCRIPTION("XPFUrandom Kernel test module");
MODULE_LICENSE("GPL");
