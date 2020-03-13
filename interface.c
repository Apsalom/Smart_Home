#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include "linux/i2c-dev.h"
#include <string.h>
#include "type.h"
#include "definition.h"

char32 path1[50];

char32 *path_to_direction(char32 *pin_num)
{
	path1[0] = '\0';
	strcat(path1, "/sys/class/gpio/gpio");
	strcat(path1, pin_num);
	strcat(path1, "/direction");
	return path1;
}

char32 *path_to_value(char32 *pin_num)
{
	path1[0] = '\0';
	strcat(path1, "/sys/class/gpio/gpio");
	strcat(path1, pin_num);
	strcat(path1, "/value");
	return path1;
}

char32 *path_to_adc(char32 *pin_num)
{
	path1[0] = '\0';
	strcat(path1, "/sys/bus/iio/devices/iio:device0/in_voltage");
	strcat(path1, pin_num);
	strcat(path1, "_raw");
	return path1;
}

s_int32 get_input(char32 *pin_num)
{
	FILE *direction, *value;
	char32 gpio_set[4] = "in";
	s_int32 state;
	direction = fopen(path_to_direction(pin_num), "w");
	fseek(direction, 0, SEEK_SET);
	fprintf(direction, "%s", gpio_set);
	fflush(direction);
	value = fopen(path_to_value(pin_num), "r");
	fscanf(value, "%d", &state);
	fflush(value);
	return state;
}

s_int32 set_output(char *pin_num, int data)
{
	FILE *value, *direction;
	direction = fopen(path_to_direction(pin_num), "w"); //setting the pin as ou$
	fprintf(direction, "out");
	fflush(direction);
	fclose(direction);
	value = fopen(path_to_value(pin_num), "w"); //setting the value to glow the$
	fprintf(value, "%d", data);
	fclose(value);
}

s_int32 get_distance()
{
	clock_t start_t, end_t, total_t;
	struct timeval start_time, end_time;
	FILE *direction, *value;
	s_int32 state;

	set_output(TRIG, HIGH);
	usleep(10);
	set_output(TRIG, LOW);

	direction = fopen("/sys/class/gpio/gpio68/direction", "w");
	fprintf(direction, "in");
	fflush(direction);
	fclose(direction);

	while (1)

	{

		value = fopen("/sys/class/gpio/gpio68/value", "r");
		fscanf(value, "%d", &state);
		fflush(value);
		fclose(value);
		if (state == HIGH)
		{
			gettimeofday(&start_time, 0);
			break;
		}
	}

	while (1)
	{
		value = fopen("/sys/class/gpio/gpio68/value", "r");
		fscanf(value, "%d", &state);
		fflush(value);
		fclose(value);
		if (state == 0)
		{
			gettimeofday(&end_time, 0);
			break;
		}
	}

	double32 distance = ((end_time.tv_sec * 1000000) + end_time.tv_usec) - ((start_time.tv_sec * 1000000) + start_time.tv_usec);
	return ((distance * VELOCITY) / HALF_DISTANCE);
}

s_int32 get_flowrate_1()
{
	s_int32 count = 0, i = 0, state, threshold_time;
	time_t presentTime;
	FILE *value, *direction;
	direction = fopen("/sys/class/gpio/gpio22/direction", "w");
	fprintf(direction, "in");
	fflush(direction);
	fclose(direction);
	time(&presentTime);
	threshold_time = presentTime + 2;
	time(&presentTime);
	while (presentTime < threshold_time)
	{
		while (1)
		{
			value = fopen("/sys/class/gpio/gpio22/value", "r");
			fscanf(value, "%d", &state);
			fflush(value);
			fclose(value);
			if (presentTime < threshold_time)
				break;
			if (state == 1)
			{
				break;
			}
		}
		while (1)
		{
			value = fopen("/sys/class/gpio/gpio22/value", "r");
			fscanf(value, "%d", &state);
			fflush(value);
			fclose(value);
			if (presentTime < threshold_time)
				break;
			if (state == 0)
			{
				count++;
				break;
			}
		}
		time(&presentTime);
	}

	return count;
}

s_int32 get_flowrate_2()
{
	s_int32 count = 0, i = 0, state, threshold_time;
	time_t presentTime;
	FILE *value, *direction;
	direction = fopen("/sys/class/gpio/gpio51/direction", "w");
	fprintf(direction, "in");
	fflush(direction);
	fclose(direction);
	time(&presentTime);
	threshold_time = presentTime + 5;
	time(&presentTime);
	while (presentTime < threshold_time)
	{
		while (1)
		{
			value = fopen("/sys/class/gpio/gpio51/value", "r");
			fscanf(value, "%d", &state);
			fflush(value);
			fclose(value);
			if (presentTime < threshold_time)
				break;
			if (state == 1)
			{
				break;
			}
		}
		while (1)
		{
			value = fopen("/sys/class/gpio/gpio51/value", "r");
			fscanf(value, "%d", &state);
			fflush(value);
			fclose(value);
			if (presentTime < threshold_time)
				break;
			if (state == 0)
			{
				count++;
				break;
			}
		}
		time(&presentTime);
	}

	return count;
}

s_int32 is_available()
{
	return get_input("61");
}

s_int32 get_moisture()
{
	FILE *adc_file;
	s_int32 adc_value;
	adc_file = fopen(path_to_adc("5"), "r");
	fscanf(adc_file, "%d", &adc_value);
	fclose(adc_file);
	return adc_value;
}

s_int32 is_it_day()
{
	FILE *adc_file;
	s_int32 adc_value;
	adc_file = fopen(path_to_adc("3"), "r");
	fscanf(adc_file, "%d", &adc_value);
	fclose(adc_file);
	return adc_value;
}

s_int32 battery_status()
{
	s_int32 i2c_dev_node = 0;
	s_int32 i2c_dev_address = 0x24;
	s_int32 i2c_dev_reg_addr = 0x0A;
	char32 i2c_dev_node_path[] = "/dev/i2c-0";
	s_int32 decimal;
	s_int32 ret_val = 0;
	__s32 read_value = 0;
	i2c_dev_node = open(i2c_dev_node_path, O_RDWR);
	if (i2c_dev_node < 0)
	{
		perror("Unable to open device node.");
		exit(1);
	}
	ret_val = ioctl(i2c_dev_node, I2C_SLAVE_FORCE, i2c_dev_address);
	if (ret_val < 0)
	{
		perror("Could not set I2C_SLAVE.");
		exit(2);
	}
	read_value = i2c_smbus_read_byte_data(i2c_dev_node, i2c_dev_reg_addr);
	if (read_value < 0)
	{
		perror("I2C Read operation failed.");
		exit(3);
	}
	decimal = read_value & 0xc0;
	if (decimal == 0)
		return 1;
	else
		return 0;
}

s_float32 get_temperature()
{
	s_int32 i2c_dev_node = 0;
	s_int32 i2c_dev_address = 0x48;			   /* LM75ADP Device Address */
	s_int32 i2c_dev_reg_addr = 0x00;		   /* Temperature Register Address */
	char32 i2c_dev_node_path[] = "/dev/i2c-2"; /* temp  Adapter Device Node*/
	s_int32 ret_val = 0;
	__s32 read_value = 0;
	/* Open the device node for the I2C adapter of bus 2 */
	i2c_dev_node = open(i2c_dev_node_path, O_RDWR);
	if (i2c_dev_node < 0)
	{
		perror("Unable to open device node.");
		exit(1);
	}
	/* Set I2C_SLAVE for adapter 2 */
	ret_val = ioctl(i2c_dev_node, I2C_SLAVE, i2c_dev_address);
	if (ret_val < 0)
	{
		perror("Could not set I2C_SLAVE.");
		exit(2);
	}
	/* Read byte from the 0x00 register of the I2C_SLAVE device */
	read_value = i2c_smbus_read_byte_data(i2c_dev_node, i2c_dev_reg_addr);
	if (read_value < 0)
	{
		perror("I2C Read operation failed.");
		exit(3);
	}

	return read_value;
}

void turn_on_motor()
{
	set_output(MOTOR_PIN, HIGH); //45
}

void turn_off_motor()
{
	set_output(MOTOR_PIN, LOW);
}

void turn_on_valve1()
{
	set_output(VALVE_1_PIN, HIGH); //66
}

void turn_off_valve1()
{
	set_output(VALVE_1_PIN, LOW);
}

void turn_on_valve2()
{
	set_output(VALVE_2_PIN, HIGH); //69
}

void turn_off_valve2()
{
	set_output(VALVE_2_PIN, LOW);
}

int switch_condition()
{
	return get_input(SWITCH_PIN); //49
}
