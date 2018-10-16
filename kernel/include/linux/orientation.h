#ifndef _LINUX_ORIENTATION_H
#define _LINUX_ORIENTATION_H

#include <linux/spinlock.h>

struct dev_orientation_k {
  int azimuth; /* angle between the magnetic north and the Y axis, around the
		* Z axis (-180<=azimuth<=180)
		*/
  int pitch;   /* rotation around the X-axis: -90<=pitch<=90 */
  int roll;    /* rotation around Y-axis: +Y == -roll, -180<=roll<=180 */
  spinlock_t orient_lock;
};

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

static struct dev_orientation_k current_orient;

#endif
