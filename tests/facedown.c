/*
 * Columbia University
 * COMS W4118 Fall 2018
 * Homework 3 - orientd.c
 * teamN: UNI, UNI, UNI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "facedown.h"

static int children_count = 3;
static int parent_life_cycle = 60;
static int child_polling_inteval = 1;

int main(int argc, char **argv)
{
  /* when phone backside faces straigh towards me, azimuth = +/-180, pitch = 0, roll = +/-180 */
  struct dev_orientation orient_facedown = {180, 0, 180};
  /* +/-5 degrees for error, roll error does not matter because it can rotate */
  struct orientation_range orient_r = {orient_facedown, 5, 5, 0};
  int event_id = orientevt_create(&orient_r);

  if(event_id < 0){
    printf("orientevt_create failed.\n");
    return EXIT_FAILURE;
  }

  for (int i=0; i<children_count; i++){
    pid_t pid = fork();

    if(pid < 0){
      printf("fork failed\n");
      orientevt_destroy(event_id);
      exit(EXIT_FAILURE);
    } else if(pid == 0) {
      /* child: wait for orient event */
      while(1){
        int wait_ret = orientevt_wait(event_id);
        if (wait_ret < 0){
	  /*event not found, exit*/
	  printf("exiting child process, pid: %d", getpid());
	  exit(EXIT_SUCCESS);
	}
	printf("%d: facing down!", i);

	/* sleep some time according to hw3 instructions */
	sleep(child_polling_inteval);
      }
    }

    /* parent process continue to create next child */
  }

  sleep(parent_life_cycle);

  /* destroying the events to kill children */
  orientevt_destroy(event_id);

  return EXIT_SUCCESS;
}
