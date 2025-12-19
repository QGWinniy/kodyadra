#include "bakery.h"
#include <stdio.h>
#include <time.h>

static int next_number = 1;
static int current_number = 1;

int *
get_ticket_number_1_svc(int *argp, struct svc_req *rqstp)
{
	static int  result;
	(void)argp;

	result = next_number;
	next_number++;
	printf("Клиент %d получил номер: %d\n", *argp, result);
	return &result;
}

quad_t *
get_service_time_1_svc(int *argp, struct svc_req *rqstp)
{
	static quad_t result;
	struct timespec ts;

	if (*argp != current_number) {
		result = -1;
		return &result;
	}
	usleep(50 * 1000);

	clock_gettime(CLOCK_REALTIME, &ts);
	long seconds_in_minute = ts.tv_sec % 60;
	long microseconds = ts.tv_nsec / 1000;
	result = (quad_t)seconds_in_minute * 1000000LL + microseconds;

	current_number++;
	return &result;
}
