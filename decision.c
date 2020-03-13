#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "control.h"
#include "type.h"
#include "definition.h"

extern time_t startTime, endTime;
time_t presentTime;

s_int32 servicing = 0;
s_int32 taskTime = 0;

void battery_decision(s_int32 servicing_interrupt)
{
	time_t stopTime = 0;
	s_int32 diff_t = 0, remaining_t = 0;

	printf("battery decision\n");
	printf("%d\n", servicing);

	if (servicing == 1)
	{
		printf("Entering if\n");
		time(&stopTime);
		printf("battery Start Time %d\n", startTime);
		printf("End Time %lf\n", stopTime);
		diff_t = difftime(stopTime, startTime);
		printf("EndTime - startTime : %lf\n", diff_t);
		remaining_t = taskTime - diff_t;
		printf("remaning time: %d\n", remaining_t);
		FILE *save;
		//	fflush(save);
		save = fopen("/home/debian/version_2/final/powerbackup.txt", "w");
		fprintf(save, "%d", remaining_t);
		fclose(save);
	}
	system("shutdown -h now");
	/*	which interrupt is serviced
 *	get the current time and save it as end time
 *	calculate the remaining time from the start and end time*/
}

void pir_decision()
{
	double32 diff_t;

	taskTime = PIR_TASKTIME;
	time(&presentTime);
	servicing = 1;
	printf("startTime : %d\n", startTime);
	printf("endTime : %d\n", endTime);
	diff_t = difftime(presentTime, endTime);
	printf("diff_t : %d\n", diff_t);
	if (diff_t > IRRIGATION_TIME_GAP)
	{
		printf("Irrigate for %lf seconds - pir\n", taskTime);
		irrigate(taskTime);
	}
	else
	{
		double32 timeIrrigated, remainingTime;
		timeIrrigated = difftime(endTime, startTime);
		remainingTime = taskTime - timeIrrigated;
		if (remainingTime > 0)
		{

			printf("Irrigate for %lf seconds remaining time - pir\n", remainingTime);
			irrigate(remainingTime);
		}
		else
		{
			printf("Do not irrigate - pir\n");
		}
	}
	servicing = 0;
}

void moisture_decision()
{
	double32 diff_t;

	taskTime = MOISTURE_TASKTIME;
	time(&presentTime);
	servicing = 1;
	diff_t = difftime(presentTime, endTime);
	if (diff_t > IRRIGATION_TIME_GAP)
	{

		printf("Irrigate for %lf seconds - moisture\n", taskTime);
		irrigate(taskTime);
	}
	else
	{
		double32 timeIrrigated, remainingTime;
		timeIrrigated = difftime(endTime, startTime);
		remainingTime = taskTime - timeIrrigated;
		if (remainingTime > 0)
		{
			printf("going to Irrigate for %lf seconds - moisture\n", remainingTime);
			irrigate(remainingTime);
		}
		else
		{
			printf("Do not irrigate - moisture\n");
		}
	}
	servicing = 0;
}

void temp_decision()
{
	double32 diff_t;

	taskTime = TEMP_TASKTIME;
	time(&presentTime);
	diff_t = difftime(presentTime, endTime);
	servicing = 1;
	if (diff_t > IRRIGATION_TIME_GAP)
	{

		printf("Irrigate for %lf seconds - temperature\n", taskTime);
		irrigate(taskTime);
	}
	else
	{
		double32 timeIrrigated, remainingTime;
		timeIrrigated = difftime(endTime, startTime);
		remainingTime = taskTime - timeIrrigated;
		if (remainingTime > 0)
		{

			printf("Irrigate for %lf seconds - temperature\n", remainingTime);
			irrigate(remainingTime);
		}
		else
		{
			printf("Do not irrigate - temperature\n");
		}
	}
	servicing = 0;
}
