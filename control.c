#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "interface.h"
#include "type.h"
#include "definition.h"

extern time_t startTime, endTime;

void fill_the_tank()
{
	s_int32 distance, count;
	printf("checking the water level in the tank\n");

	sleep(2);
	distance = get_distance();
	printf("distance is %d\n", distance);

	if (distance > WATER_LOW_MARK)
	{
		printf("filling water\n");

		turn_on_motor();
		count = get_flowrate_1();

		if (count == 0)
		{
			printf("water is not in the source\n");
			turn_off_motor();
		}
		else
		{
			while (1)
			{
				sleep(2); /* wait time for ultrasonic consecutive read */
				distance = get_distance();
				printf("distance is %d\n", distance);

				if (distance <= WATER_UP_MARK)
				{
					printf("filled the watertank\n");
					turn_off_motor();
					break;
				}
			}
		}
	}
}

void irrigate(s_int32 time_irr)
{
	int count;

	printf("inside Irrigate functio : %d\n", time_irr);
	fill_the_tank();
	printf("checking switch condition \n");

	if (switch_condition())
	{
		printf("starting the irrigation\n");
		time(&startTime);

		turn_on_valve1();
		turn_on_valve2();

		if (time_irr > 4)
		{
			time_irr = time_irr - 5;
			get_flowrate_2();
		}
		sleep(time_irr);

		turn_off_valve1();
		turn_off_valve2();

		time(&endTime);

		printf("Exiting irrigate\n");
	}
}
