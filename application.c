#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include "decision.h"
#include "interface.h"
#include "control.h"
#include "type.h"
#include "definition.h"

time_t startTime, endTime;

u_int32 interrupt_counter = 0;

s_int32 thresh_moisture = 2000;
s_float32 initialTemp;

static struct mq_attr my_mq_attr1;
static struct mq_attr my_mq_attr2;

static mqd_t my_mq1;
static mqd_t my_mq2;

s_int32 recv_counter1 = 0;

u_int32 counter1 = 0;
u_int32 counter2 = 0;

pthread_t thread1, thread2;

void sig_handler(s_int32 signum)
{
	if (signum != SIGINT)
	{
		printf("Received invalid signum = %d in sig_handler()\n", signum);
		usleep(100);
	}
	printf("Received SIGINT. Exiting Application\n");
	pthread_cancel(thread1);
	pthread_cancel(thread2);
	/*  FILE *save;											// clear the file when the program is manualy terminated 
	    save = fopen("/home/debian/version_2/final/powerbackup.txt","w");
	    fclose(save);*/
	mq_close(my_mq1);
	mq_close(my_mq2);
	mq_unlink("/my_mq1");
	mq_unlink("/my_mq2");
	battery_decision();
	exit(0);
}

void initial()
{
	s_int32 remainingTime = 0;
	s_int32 lSize;
	FILE *save = fopen("/home/debian/version_2/final/powerbackup.txt", "r");
	if (save) 												//checking for the emptiness in the file
	{
		fseek(save, 0, SEEK_END);
		lSize = ftell(save);
		fseek(save, 0, SEEK_SET);
		printf("file length %d\n", lSize);
		if (lSize != 0)
		{
			printf("file is not empty\n");
			fscanf(save, "%d", &remainingTime);
			printf("remaining file %d\n", remainingTime);
			irrigate(remainingTime);
		}
		else
			printf("file is empty\n");
		fclose(save);
	}
	fill_the_tank();
}

void pir_handler()
{
	counter1 = PIR_TIMER_IDENTIFIER;
	printf("pir interrupt from 24 hour timer \n\n");
	mq_send(my_mq1, (const char *)&counter1, sizeof(counter1), 1);
}

void *task1()
{
	printf("Entering task 1 \n");
	s_int32 moisture, ldr = 0;
	s_int32 pir_timer_enable = 0;
	s_int32 curr_temperature;

	struct sigevent sigeven5;
	struct itimerspec trigger5;

	s_int32 recv_counter = 0;

	timer_t timer5;

	memset(&sigeven5, 0, sizeof(struct sigevent));
	memset(&trigger5, 0, sizeof(struct itimerspec));

	sigeven5.sigev_notify = SIGEV_THREAD;
	sigeven5.sigev_notify_function = &pir_handler;

	timer_create(CLOCK_REALTIME, &sigeven5, &timer5);

	for (;;)
	{
		if (is_it_day() > 1500)
		{

			if (ldr == 0)
			{
				printf("it is day\n");
				fill_the_tank();
				ldr = 1;
			}
		}

		if (is_it_day() < 1500)
		{
			if (ldr == 1)
			{
				printf("it is night\n");
				ldr = 0;
			}
		}

		mq_receive(my_mq1, (char *)&recv_counter, sizeof(recv_counter), NULL);
		printf("received\n");
		if (is_available() == 1) 							//person is not available
		{
			printf("person is not availble\n");
			if (pir_timer_enable == 0)
			{
				timer_create(CLOCK_REALTIME, &sigeven5, &timer5);
				trigger5.it_value.tv_sec = 180; 			//timer for pir sensor irrigation
				trigger5.it_interval.tv_sec = 180;
				printf("24 HOURS TIMER\n");
				timer_settime(timer5, 0, &trigger5, NULL);
				pir_timer_enable = 1;
			}
		}

		else
		{
			printf("the person is available\n");
			timer_delete(timer5);
			pir_timer_enable = 0;
		}
		printf("Interrupt countr %d\n", interrupt_counter);

		if (recv_counter == 0)
		{
			pir_decision();
		}

		else if (recv_counter % 60 == 0) 					/*check for temperature interrupt*/
		{
			printf("temperature case \n\n");
			curr_temperature = get_temperature();
			printf("temperature %d\n", curr_temperature);
			if (curr_temperature > 10)
				temp_decision();
			else
				printf("temperature difference is less than 10\n");
			initialTemp = curr_temperature;
		}

		else if (recv_counter % 15 == 0)					/*check for moisture interrupt*/
		{
			printf("moisture case \n\n");
			moisture = get_moisture();
			printf("moisture value: %d\n", moisture);
			if (moisture < thresh_moisture)
				moisture_decision();
			else
				printf("Value greater than threshold value\n");
		}
	}
}

void *battery_task()
{
	s_int32 recv_counter2 = 0;
	s_int32 status = 0;
	while (1)
	{
		mq_receive(my_mq2, (char32 *)&recv_counter2, sizeof(recv_counter2), NULL);
		if (recv_counter2 == 4)
			;
		{
			printf("entered the battery task\n");
			status = battery_status();
			if (status == 1)
				;
			//battery_decision();
		}
	}
}

void handler()
{
	counter2 = BATTERY_IDENTIFIER;
	++interrupt_counter;
	printf("pir interrupt \n\n");
	printf("Count%d\n", interrupt_counter);
	mq_send(my_mq1, (const char32 *)&interrupt_counter, sizeof(interrupt_counter), 1);
	if (interrupt_counter % 3 == 0)
	{
		printf("battery interrupt \n\n");
		mq_send(my_mq2, (const char32 *)&counter2, sizeof(counter2), 1);
	}
}

void main()
{
	struct sigevent sigeven, sigeven2, sigeven3, sigeven4;
	struct itimerspec trigger, trigger2, trigger3, trigger4;
	timer_t timer1, timer2, timer3, timer4;

	signal(SIGINT, sig_handler);

	initial();
	printf("process started\n");

	initialTemp = get_temperature();

	/*initializing structure*/
	memset(&sigeven, 0, sizeof(struct sigevent));
	memset(&trigger, 0, sizeof(struct itimerspec));

	my_mq_attr1.mq_maxmsg = MAX_MESSAGES;
	my_mq_attr1.mq_msgsize = sizeof(counter1);

	my_mq_attr2.mq_maxmsg = MAX_MESSAGES;
	my_mq_attr2.mq_msgsize = sizeof(counter2);

	my_mq1 = mq_open("/my_mq1", O_CREAT | O_RDWR | 0, 0666, &my_mq_attr1);
	my_mq2 = mq_open("/my_mq2", O_CREAT | O_RDWR | 0, 0666, &my_mq_attr2);

	/*handler for interrupt*/
	sigeven.sigev_notify = SIGEV_THREAD;
	sigeven.sigev_notify_function = &handler;

	/*creating Timer*/
	timer_create(CLOCK_REALTIME, &sigeven, &timer1);

	trigger.it_value.tv_sec = PIR_INTERVAL; //timer for pir sensor
	trigger.it_interval.tv_sec = PIR_INTERVAL;

	timer_settime(timer1, 0, &trigger, NULL);

	pthread_create(&thread1, NULL, (void *)&task1, NULL);
	pthread_create(&thread2, NULL, (void *)&battery_task, NULL);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
}
