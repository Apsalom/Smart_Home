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


char path1[50];

char *path_to_direction(char *pin_num)
{
    path1[0]='\0';
    strcat(path1, "/sys/class/gpio/gpio");
    strcat(path1, pin_num);
    strcat(path1, "/direction");
    return path1;
}

char *path_to_value(char *pin_num)
{
    path1[0]='\0';
    strcat(path1, "/sys/class/gpio/gpio");
    strcat(path1, pin_num);
    strcat(path1, "/value");
    return path1;
}

char *path_to_adc(char *pin_num)
{
    path1[0]='\0';
    strcat(path1, "/sys/bus/iio/devices/iio:device0/in_voltage");
    strcat(path1, pin_num);
    strcat(path1, "_raw");
    return path1;
}

int get_input(char *pin_num)
{
    FILE *direction, *value;
    char gpio_set[4] = "in";
    int state;
    direction = fopen(path_to_direction(pin_num), "w");
    fseek(direction, 0, SEEK_SET);
    fprintf(direction, "%s", gpio_set);
    fflush(direction);
    value = fopen(path_to_value(pin_num), "r");
    fscanf(value, "%d", &state);
    fflush(value);
    return state;
}


int set_output(char *pin_num, int data)
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


int get_distance()
{
	clock_t start_t, end_t, total_t;
	struct timeval start_time,end_time;
	FILE *direction, *value;
	int state;
	
	set_output("67",1);
	set_output("67",0);
	
	direction = fopen("/sys/class/gpio/gpio68/direction", "w");
	fprintf(direction, "in");
	fflush(direction);
	fclose(direction);
	
	while(1)
	
	{

		value = fopen("/sys/class/gpio/gpio68/value", "r");
		fscanf(value, "%d", &state);
		fflush(value);
		if(state==1)
		{
			gettimeofday(&start_time,0);
			fclose(value);
			break;
		}
		fclose(value);
	}

	while(1)
	{


		value = fopen("/sys/class/gpio/gpio68/value", "r");
		fscanf(value, "%d", &state);
		fflush(value);
		if(state == 0)
		{
			gettimeofday(&end_time,0);
			fclose(value);
			break;
		}
		fclose(value);
	}

	double  distance =((end_time.tv_sec*1000000)+end_time.tv_usec)-((start_time.tv_sec*1000000)+start_time.tv_usec);
	return ((distance*0.034)/2);
}



int get_flowrate_1()
{
    return get_input("22");
}

int get_flowrate_2()
{
    return get_input("51");
}

int is_available()
{
	return get_input("61");
}

int get_moisture()
{
    FILE *adc_file;
    int adc_value;
    adc_file = fopen(path_to_adc("5"), "r");
    fscanf(adc_file, "%d", &adc_value);
    fclose(adc_file);
    return adc_value;
}

int is_it_day()
{
    FILE *adc_file;
    int adc_value;
    adc_file = fopen(path_to_adc("3"), "r");
    fscanf(adc_file, "%d", &adc_value);
    fclose(adc_file);
    return adc_value;
}


int battery_status()
{
	int i2c_dev_node = 0;
	int i2c_dev_address = 0x24;
        int i2c_dev_reg_addr = 0x0A;
	char i2c_dev_node_path[] = "/dev/i2c-0";
	int decimal;
	int ret_val = 0;
	__s32 read_value = 0;
	i2c_dev_node = open(i2c_dev_node_path,O_RDWR);	
	if (i2c_dev_node < 0)
	 {
	         perror("Unable to open device node.");
	         exit(1);
	 }
	ret_val = ioctl(i2c_dev_node,I2C_TENBIT, i2c_dev_address);
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
	decimal=read_value&0x000000c0;
	printf("%x\n%x\n",read_value,decimal);
	if(decimal==1)
		return 1;
	else 
		return 0;
}


float get_temperature()
{
	 int i2c_dev_node = 0;
	 int i2c_dev_address = 0x48;              /* LM75ADP Device Address */
	 int i2c_dev_reg_addr = 0x00;             /* Temperature Register Address */
	 char i2c_dev_node_path[] = "/dev/i2c-2"; /* temp  Adapter Device Node*/
         int ret_val = 0;
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
   /* Read the temperature from the sensor */
   /* Mask the upper 3 bits as they do not pertain to measurement*/
   /* read_value &= 0x1F;
    *
   * 		if (read_value < 0) {
    * 					perror("I2C Read operation failed.");
    * 								exit(3);
   }*/
     // printf("temperature Val = %X\n", read_value);
      return read_value;
}



void turn_on_motor()
{
    set_output("46", 1);
}

void turn_off_motor()
{
    set_output("46", 0);
}

void turn_on_valve1()
{
    set_output("66", 1);
}

void turn_off_valve1()
{
    set_output("66", 0);
}

void turn_on_valve2()
{
    set_output("69", 1);
}

void turn_off_valve2()
{
    set_output("69", 0);
}
