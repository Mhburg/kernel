/* orientation related syscalls */ 
#include <linux/orientation.h>
#include <linux/syscalls.h>
// #include <linux/spinlock.h>

static void __print_orient_k(void){
  printk(KERN_ERR "Gang: printing current orient in kernel.\n");
  printk(KERN_ERR "Gang: azimuth: %d\n", current_orient.azimuth);
  printk(KERN_ERR "Gang: pitch: %d\n", current_orient.pitch);
  printk(KERN_ERR "Gang: roll: %d\n", current_orient.roll);
}


SYSCALL_DEFINE1(set_orientation, struct dev_orientation __user *, orient)
{
  printk(KERN_ERR "Gang: set_orientation starts.\n");
  spin_lock(&(current_orient.orient_lock));

  if (copy_from_user(&(current_orient.azimuth), &(orient->azimuth), sizeof(int)) > 0){
    printk(KERN_ERR "Gang: copy from user error 1.\n");
    return -EFAULT;
  }
  if (copy_from_user(&(current_orient.pitch), &(orient->pitch), sizeof(int)) > 0){
    printk(KERN_ERR "Gang: copy from user error 2.\n");
    return -EFAULT;
  }
  if (copy_from_user(&(current_orient.roll), &(orient->roll), sizeof(int)) > 0){
    printk(KERN_ERR "Gang: copy from user error 3.\n");
    return -EFAULT;
  }
  spin_unlock(&(current_orient.orient_lock));
  // __print_orient_k();
  return 23;
}
