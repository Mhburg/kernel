/* orientation related syscalls */ 
#include <linux/orientation.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <asm/atomic.h>



// static void __print_orient_k(void){
//   printk(KERN_ERR "Gang: printing current orient in kernel.\n");
//   printk(KERN_ERR "Gang: azimuth: %d\n", current_orient.azimuth);
//   printk(KERN_ERR "Gang: pitch: %d\n", current_orient.pitch);
//   printk(KERN_ERR "Gang: roll: %d\n", current_orient.roll);
// }

static __always_inline bool orient_within_range(struct dev_orientation *orient,
                                struct orientation_range *range);

SYSCALL_DEFINE1(set_orientation, struct dev_orientation __user *, orient)
{
  printk(KERN_ERR "Gang: set_orientation starts.\n");
  // write_lock(&(orient_lock));

  // if (copy_from_user(&(current_orient.azimuth), &(orient->azimuth), sizeof(int)) > 0){
  //   printk(KERN_ERR "Gang: copy from user error 1.\n");
  //   return -EFAULT;
  // }
  // if (copy_from_user(&(current_orient.pitch), &(orient->pitch), sizeof(int)) > 0){
  //   printk(KERN_ERR "Gang: copy from user error 2.\n");
  //   return -EFAULT;
  // }
  // if (copy_from_user(&(current_orient.roll), &(orient->roll), sizeof(int)) > 0){
  //   printk(KERN_ERR "Gang: copy from user error 3.\n");
  //   return -EFAULT;
  // }
  // write_unlock(&(orient_lock));
  // // __print_orient_k();

  /* new implementation */
  if (kfifo_from_user(&orient_fifo, &orient, sizeof(struct dev_orientation), (void *)0) == -EFAULT){
    printk(KERN_ERR "Zizhen: copy from user error 4.\n");
    return -EFAULT;
  }

  if (!atomic64_read(&orient_reader)){
      spin_lock(&orient_reader_lock);
      if (!atomic64_read(&orient_reader)){
          pid_t tmp = sys_fork();
          if (tmp < 0){
              return -ECHILD;
          } else if (tmp > 0){
              int ret;
              atomic64_set(&orient_reader, tmp);
              ret = kfifo_alloc(&orient_fifo, QUEUE_SIZE, GFP_KERNEL);
              if (ret)
                  return ret;
              evt_head = (struct list_head){ &(evt_head), &(evt_head) };
              // evt_head = LIST_HEAD_INIT(evt_head);
              rwlock_init(evt_head_lock);



              
              return 0;
          } else {
              /* in child process */
              /* make if daemon */
              if (sys_setsid() < 0)
                  return -ESRCH;

              struct dev_orientation *orient = (struct dev_orientation *)kmalloc(sizeof(struct dev_orientation), GFP_KERNEL);
              /* if there is no event in queue, orient_reader will be set to 0 */
              while (atomic64_read(&orient_reader)){
                  while (kfifo_avail(&orient_fifo)){
                      struct event_t *curr;
                      if (kfifo_out(&orient_fifo, orient, sizeof(struct dev_orientation)) != sizeof(struct dev_orientation))
                          return -EINVAL;

                      read_lock(evt_head_lock);
                      list_for_each_entry(curr, &evt_head, head){
                          read_lock(curr->rwlock);
                          if (orient_within_range(orient, &curr->range))
                              wake_up(curr->wq);
                          read_unlock(curr->rwlock);
                      }
                      read_unlock(evt_head_lock);
                  }
              }

              kfree(orient);

              return 0;
          } /* child process ends */
      }
  }



  return 0;
}


SYSCALL_DEFINE1(orientevt_create, struct orientation_range __user *, orient){
  struct event_t *evt = (struct event_t *)kmalloc(sizeof(struct event_t), GFP_KERNEL);
  printk(KERN_ERR "Gang: orientevt_create done.\n");

  if (evt)
      return -ENOMEM;
  if (copy_from_user(&evt->range, orient, sizeof(struct orientation_range)) > 0)
      return -EINVAL;

  struct event_t *curr;
  write_lock(evt_head_lock);
  list_for_each_entry(curr, &evt_head, head){
      write_lock(curr->rwlock);
      if (event_equal(&evt->range, &curr->range)){
         int id  = ++curr->ref_count;
         write_unlock(curr->rwlock);
         write_unlock(evt_head_lock);
         return id;
      } 
      write_unlock(curr->rwlock);
  }
  
  /* evt initialization */
  struct event_t *prev = list_prev_entry(evt, head);
  if (!prev){
      evt->id = 1;
  } else {
      read_lock(prev->rwlock);
      evt->id = prev->id + 1;
      read_unlock(prev->rwlock);
  }

  evt->destory = 0;
  evt->ref_count = 0;
  evt->head = (struct list_head){ &(evt->head), &(evt->head) };
  init_waitqueue_head(evt->wq);
  rwlock_init(evt->rwlock);

  list_add(&evt->head, &evt_head);
  int id = evt->id;
  write_unlock(evt_head_lock);

  return id;
}


SYSCALL_DEFINE1(orientevt_destroy, int, event_id){
  printk(KERN_ERR "Gang: orientevt_destroy done.\n");
  int id;
  if (copy_from_user(&id, &event_id, sizeof(int)) > 0){
     return -EFAULT;
  } 

  struct event_t *curr;
  read_lock(evt_head_lock);
  list_for_each_entry(curr, &evt_head, head){
      if (id == curr->id){
          write_lock(curr->rwlock);
          --curr->ref_count;
          if (curr->destory == 0 && curr->ref_count == 0){
              write_unlock(curr->rwlock);
              /* maintain the order when acquiring locks */
              write_lock(evt_head_lock);
              write_lock(curr->rwlock);
              /* check again in case orientevt_create is called */
              if (curr->destory == 0 && curr->ref_count == 0){
                  rwlock_t *tmplock = curr->rwlock;
                  list_del(&(curr)->head);
                  write_unlock(evt_head_lock);
                  kfree(curr);
                  write_unlock(tmplock);
                  return 0;
              }
          }
          write_unlock(curr->rwlock);
          return 0;
      }
  }
  return -ENOENT;
}

SYSCALL_DEFINE1(orientevt_wait, int, event_id){
  int id;
  printk(KERN_ERR "Gang: orientevt_wait done.\n");

  if (copy_from_user(&id, &event_id, sizeof(int)) > 0){
     return -EFAULT;
  } 

  DEFINE_WAIT(wait);
  struct event_t *curr;
  read_lock(evt_head_lock);
  list_for_each_entry(curr, &evt_head, head){
      if (id == curr->id){
         read_unlock(evt_head_lock);
         write_lock(curr->rwlock);
         ++curr->ref_count;
         write_unlock(curr->rwlock); 
         prepare_to_wait(curr->wq, &wait, TASK_INTERRUPTIBLE);
         if (curr->destory == 0){
             schedule();
             return 0;
         } else {
             return 1;
         }
      }
  }

  read_unlock(evt_head_lock);
  return -1;
}

static __always_inline bool orient_within_range(struct dev_orientation *orient,
                                struct orientation_range *range)
{
    struct dev_orientation *target = &range->orient;
    unsigned int azimuth_diff = abs(target->azimuth - orient->azimuth);
    unsigned int pitch_diff = abs(target->pitch - orient->pitch);
    unsigned int roll_diff = abs(target->roll - orient->roll);
    
    return (!range->azimuth_range || azimuth_diff <= range->azimuth_range
            || 360 - azimuth_diff <= range->azimuth_range)
        && (!range->pitch_range || pitch_diff <= range->pitch_range)
        && (!range->roll_range || roll_diff <= range->roll_range
                || 360 - roll_diff <= range->roll_range);
}

static __always_inline int event_equal(struct orientation_range *new,
                                       struct orientation_range *old){
    return new->azimuth_range == old->azimuth_range
        && new->pitch_range == old->pitch_range
        && new->roll_range == old->roll_range
        && new->orient.azimuth == old->orient.azimuth
        && new->orient.pitch == old->orient.pitch
        && new->orient.roll == old->orient.roll;
}
