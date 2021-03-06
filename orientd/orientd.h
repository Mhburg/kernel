/*
 * Columbia University
 * COMS W4118 Fall 2018
 * Homework 3 - orientd.h
 * teamN: UNI, UNI, UNI
 */

#ifndef _ORIENTD_H
#define _ORIENTD_H

#include <unistd.h>

#define __NR_set_orientation 326
#define __NR_orientevt_create 327
#define __NR_orientevt_destroy 328
#define __NR_orientevt_wait 329

struct dev_orientation {
	int azimuth; /* angle between the magnetic north and the Y axis, around
		      * the Z axis (-180<=azimuth<180)
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

/* syscall wrapper */
static inline int set_orientation(struct dev_orientation *orient)
{
	return syscall(__NR_set_orientation, orient);
}

static inline int orientevt_create(struct orientation_range *orient)
{
  return syscall(__NR_orientevt_create, orient);
}

static inline int orientevt_destroy(int event_id)
{
  return syscall(__NR_orientevt_destroy, event_id);
}

static inline int orientevt_wait(int event_id)
{
  return syscall(__NR_orientevt_wait, event_id);
}

#endif /* _ORIENTD_H */
