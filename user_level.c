#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#define OFF     _IOR('q', 0, led_t *)
#define ON      _IOR('q', 1, led_t *)
#define BLINK   _IOR('q', 2, led_t *)
#define SUCCESS 0
#define FAILURE -1
#define MODE1   1
#define MODE2   2
#define MODE3   3

typedef struct{
    int status, rate;
}led_t;
led_t led_dev;

int set_blink(int fd){
    int var;
    printf("enter the blink rate\n");
    if(!scanf("%d", &var)){
        return FAILURE;
    }
    getchar();
    led_dev.status = BLINK;
    led_dev.rate = var;
    if(ioctl(fd, BLINK, &led_dev) == FAILURE){
        perror("blink failed\n");
    }
    return SUCCESS;
}

int turn_off(int fd){
    led_dev.status = OFF;
    if(ioctl(fd, OFF, &led_dev) == FAILURE){
        perror("turn off failed\n");
    }
}

int turn_on(int fd){
    led_dev.status = ON;
    if(ioctl(fd, ON, &led_dev) == FAILURE){
        perror("turn on failed\n");
    }
}

char *file_name = "/dev/led_driver";

int main()
{
    int sw;
    int fd;
    fd = open(file_name, O_RDWR);
    if(fd == FAILURE){
        perror("No file\n");
        return FAILURE;
    }
    printf("1. OFF\n2. ON\n3. BLINK\n4. EXIT\n");
    if(!scanf("%d", &sw)){
        return FAILURE;
    }
    switch(sw){
            case MODE1:
                turn_off(fd);
                break;
            case MODE2:
                turn_on(fd);
                break;
            case MODE3:
                set_blink(fd);
                break;
            default:
                printf("EXIT\n");
                return SUCCESS;
        }
    return SUCCESS;
}
