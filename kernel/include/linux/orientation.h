#ifndef _LINUX_ORIENTATION_H
#define _LINUX_ORIENTATION_H

#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/kfifo.h>
#include <linux/list.h>
#include <linux/types.h>

#define QUEUE_SIZE (2 ^ 10)

atomic64_t orient_reader;
DEFINE_SPINLOCK(orient_reader_lock);


// 
// struct dev_orientation_k {
//   int azimuth; /* angle between the magnetic north and the Y axis, around the
// 		* Z axis (-180<=azimuth<=180)
// 		*/
//   int pitch;   /* rotation around the X-axis: -90<=pitch<=90 */
//   int roll;    /* rotation around Y-axis: +Y == -roll, -180<=roll<=180 */
//   spinlock_t orient_lock;
// };

struct dev_orientation {
  int azimuth; /* angle between the magnetic north and the Y axis, around the
                * Z axis (-180<=azimuth<=180)
                */
  int pitch;   /* rotation around the X-axis: -90<=pitch<=90 */
  int roll;    /* rotation around Y-axis: +Y == -roll, -180<=roll<=180 */
};

struct orientation_range {
  struct dev_orientation orient;  /* device orientation */
  unsigned int azimuth_range;     /* +/- degrees around Z-axis */
  unsigned int pitch_range;       /* +/- degrees around X-axis */
  unsigned int roll_range;        /* +/- degrees around Y-axis */
};


struct event_t{
    unsigned int id;
    unsigned int destory;
    unsigned int ref_count;
    struct list_head head;
    struct orientation_range range;
    wait_queue_head_t *wq;
    rwlock_t *rwlock;
};

/* orientation data queue */
// static kfifo orient_fifo;
static DECLARE_KFIFO(orient_fifo, struct dev_orientation, QUEUE_SIZE);
INIT_KFIFO(orient_fifo);

/* event linked list */
static LIST_HEAD(evt_head);
static DEFINE_RWLOCK(evt_head_lock);

static __always_inline int event_equal(struct orientation_range *, struct orientation_range *);

#endif
