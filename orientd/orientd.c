/*
 * Columbia University
 * COMS W4118 Fall 2018
 * Homework 3 - orientd.c
 * teamN: UNI, UNI, UNI
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <hardware/hardware.h>
#include <hardware/sensors.h>

#include "orientd.h"

static int open_sensors(struct sensors_module_t **sensors,
			struct sensors_poll_device_t **device);
static int poll_sensor_data(struct sensors_poll_device_t *device,
			    struct dev_orientation *cur_orientation);

int main(int argc, char **argv)
{
	struct sensors_module_t *sensors;
	struct sensors_poll_device_t *device;

	if (open_sensors(&sensors, &device))
		return EXIT_FAILURE;

	/********** Demo code begins **********/
	/* Replace demo code with your daemon */
	pid_t pid, sid;

	pid = fork();
	if (pid < 0){
          /* fork() failed */
	  exit(EXIT_FAILURE);
	} else if (pid > 0){
	  /* parent process exits */
	  exit(EXIT_SUCCESS);
	}

	umask(0);
	char logfile[] = "/data/misc/orientd.log";
	FILE *f = fopen(logfile, "a");

	/*create a new session ID for child */
	sid = setsid();
	if(sid < 0){
	  fclose(f);
	  exit(EXIT_FAILURE);
	}

	int chdir_ret = chdir("/");
	if(chdir_ret < 0){
	  fclose(f);
	  printf("chdir failed, ret=%d\n", chdir_ret);
	  exit(EXIT_FAILURE);
	}

	/* syscall 327, 328, 329 test */
	struct orientation_range orient_r;
	int orientevt_create_ret = orientevt_create(&orient_r);
	int orientevt_destroy_ret = orientevt_destroy(0);
	int orientevt_wait_ret = orientevt_wait(0);
	fprintf(f, "orientevt create ret: %d\n", orientevt_create_ret);
	fprintf(f, "orientevt destroy ret: %d\n", orientevt_destroy_ret);
	fprintf(f, "orientevt wait ret: %d\n", orientevt_wait_ret);

	struct dev_orientation orientation;

	while (true) {
		if (poll_sensor_data(device, &orientation)) {
		  fprintf(f, "No data received!\n");
		} else {
		  fprintf(f, "azimuth = %d, pitch = %d, roll = %d\n",
			       orientation.azimuth, orientation.pitch,
			       orientation.roll);
		}
		usleep(100000);
		int ret = set_orientation(&orientation);
		fprintf(f,"set_orientation ret : %d\n", ret);
	}

	/*********** Demo code ends ***********/
	fclose(f);
	return EXIT_SUCCESS;
}

/***************************** DO NOT TOUCH BELOW *****************************/

static inline int open_sensors(struct sensors_module_t **sensors,
			       struct sensors_poll_device_t **device)
{
	int err = hw_get_module("sensors",
				(const struct hw_module_t **)sensors);

	if (err) {
		fprintf(stderr, "Failed to load %s module: %s\n",
			SENSORS_HARDWARE_MODULE_ID, strerror(-err));
		return -1;
	}

	err = sensors_open(&(*sensors)->common, device);

	if (err) {
		fprintf(stderr, "Failed to open device for module %s: %s\n",
			SENSORS_HARDWARE_MODULE_ID, strerror(-err));
		return -1;
	}

	if (!*device)
		return -1;

	const struct sensor_t *list;
	int count = (*sensors)->get_sensors_list(*sensors, &list);

	if (count < 1 || !list) {
		fprintf(stderr, "No sensors!\n");
		return -1;
	}

	for (int i = 0 ; i < count ; ++i) {
		printf("%s (%s) v%d\n", list[i].name, list[i].vendor,
			list[i].version);
		printf("\tHandle:%d, type:%d, max:%0.2f, resolution:%0.2f\n",
			list[i].handle, list[i].type, list[i].maxRange,
			list[i].resolution);
		(*device)->activate(*device, list[i].handle, 1);
	}
	return 0;
}

static int poll_sensor_data(struct sensors_poll_device_t *device,
			    struct dev_orientation *cur_orientation)
{
	static const size_t event_buffer_size = 128;
	sensors_event_t buffer[event_buffer_size];

	int count = device->poll(device, buffer, event_buffer_size);

	for (int i = 0; i < count; ++i) {
		if (buffer[i].type != SENSOR_TYPE_ORIENTATION)
			continue;
		cur_orientation->azimuth =
			(int)(buffer[i].orientation.azimuth / M_PI * 180);
		cur_orientation->pitch =
			(int)(buffer[i].orientation.pitch / M_PI * 180);
		cur_orientation->roll =
			(int)(buffer[i].orientation.roll / M_PI * 180);
		return 0;
	}

	return -1;
}
