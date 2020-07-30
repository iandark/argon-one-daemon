/*
MIT License

Copyright (c) 2020 DarkElvenAngel

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/gpio.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <dirent.h>
#include <poll.h>
#include <inttypes.h>
#include <time.h>
#include <ctype.h>
#include "event_timer.h"
#include "identapi.h"
#include "argononed.h"

#define VERSION "0.2.0 experimental"
#ifndef LOG_LEVEL
#define LOG_LEVEL 6
#endif

char* PI_PUD_STR[3] = {"OFF", "DOWN", "UP"};
char* PI_MODE_STR[8] = { "INPUT", "OUTPUT", "ALT5", "ALT4", "ALT0", "ALT1", "ALT2", "ALT3" };
char* LOG_LEVEL_STR[6] = {"FATAL", "CRITICAL", "ERROR", "WARNING", "INFO",  "DEBUG"};

uint8_t fanstage[4] = { 10 ,55, 100, 0 };
uint8_t threshold[4] = { 55, 60, 65, 0 };
uint8_t hysteresis = 3;
uint32_t *gpioReg = MAP_FAILED;
int  runstate = 0;
struct SHM_Data* ptr;
typedef enum {
    LOG_NONE = 0,
    LOG_FATAL,
    LOG_CRITICAL,
    LOG_ERROR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG,
} Log_Level;


//void log_message(const char *level, const char *message, ...)
void log_message(Log_Level level, const char *message, ...)
{
   FILE *logfile;
   va_list args;
   logfile = fopen(LOG_FILE,"a");
   if(!logfile)
      return;
   if (level > LOG_LEVEL ) return;
   time_t now;
   time(&now);
   char * date = ctime(&now);
   date[strlen(date) - 1] = '\0';
   fprintf(logfile,"%s [%s] ", date, LOG_LEVEL_STR[level - 1]);
   va_start(args, message);
   vfprintf(logfile, message, args);
   va_end(args);
   fprintf(logfile,"\n");
   fclose(logfile);
}

void cleanup()
{
   log_message(LOG_INFO, "Cleaning up");
   TMR_Get_temp(0,"0");
   Set_FanSpeed(0);
   Set_FanSpeed(0xFF);
   close_timers();
   shm_unlink(SHM_FILE);
   unlink(LOCK_FILE);
   log_message(LOG_INFO, "Ready for shutdown");
}

void signal_handler(int sig){
    switch(sig){
    case SIGHUP:
        log_message(LOG_INFO,"Hangup Signal - RELOAD CONFIG");
        reload_config_from_shm();
        break;
    case SIGTERM:
        log_message(LOG_INFO,"Terminate Signal");
        cleanup();
        log_message(LOG_INFO,"Exiting");
        exit(0);
        break;
    }
}

void Read_config()
{
    FILE *fp = NULL;
    uint32_t ret = 0;
    struct DTBO_Config datain = {0};
    log_message(LOG_INFO,"Reading values from device-tree");
    fp = fopen("/proc/device-tree/argonone/argonone-cfg","rb");
    if (fp == NULL)
    {
        log_message(LOG_ERROR,"Unable to open device-tree data");
    } else {
        ret = fread(&datain,sizeof(struct DTBO_Config),1,fp);
        if (ret <= 0)
        {
            log_message(LOG_ERROR,"Unable to read device-tree data");
        } else {
            if (datain.hysteresis <= 10) hysteresis = datain.hysteresis;
            for (int i = 0; i < 3; i++)
            {
                if (i > 0)
                {
                    if (datain.fanstages[i] > datain.fanstages[i - 1])
                        datain.fanstages[i] = fanstage[i] = datain.fanstages[i] <= 100 ? datain.fanstages[i] : fanstage[i];
                    if (datain.thresholds[i] > datain.thresholds[i - 1])
                        datain.thresholds[i] = threshold[i] = datain.thresholds[i] <= 80 ? datain.thresholds[i] : threshold[i];
                } else {
                    datain.fanstages[i] = fanstage[i] = datain.fanstages[i] <= 100 ? datain.fanstages[i] : fanstage[i];
                    datain.thresholds[i] =threshold[i] = datain.thresholds[i] <= 80 ? datain.thresholds[i] : threshold[i];
                }
            }
            log_message(LOG_INFO,"Hysteresis set to %d",hysteresis);
            log_message(LOG_INFO,"Fan Speeds set to %d%% %d%% %d%%",fanstage[0],fanstage[1],fanstage[2]);
            log_message(LOG_INFO,"Fan Temps set to %d %d %d",threshold[0],threshold[1],threshold[2]);
        }
        fclose(fp);
    }
}

char* RUN_STATE_STR[4] = {"AUTO", "OFF", "MANUAL", "COOLDOWN"};
void reload_config_from_shm()
{
    for (int i = 0; i < 3; i++)
    {
        int lastval = 30;
        if (ptr->config.thresholds[i] < lastval)
        {
            log_message(LOG_WARN,"Shared Memory contains bad value at threshold %d ABORTING reload", i);
            return;
        }
    }
    for (int i = 0; i < 3; i++)
    {
        if (ptr->config.fanstages[i] > 100 )
        {
            log_message(LOG_WARN,"Shared Memory contains bad value at fanstage %d ABORTING reload", i);
            return;
        }
    }
    memcpy(&fanstage, ptr->config.fanstages, sizeof(fanstage));
    memcpy(&threshold, ptr->config.thresholds, sizeof(threshold));
    if (ptr->config.hysteresis > 10)
    {
        log_message(LOG_WARN,"Shared Memory contains bad value at hysteresis FORCING to 10");
        ptr->config.hysteresis = 10;
    }
    hysteresis = ptr->config.hysteresis;
    if (ptr->fanmode > 3) 
    {
        log_message(LOG_WARN,"Shared Memory contains bad value at fanmode FORCING to AUTO");
        ptr->fanmode = 0;
    }
    runstate = ptr->fanmode;
    threshold[3] = ptr->temperature_target >= 30 ? ptr->temperature_target : 30;
    fanstage[3]  = ptr->fanspeed_Overide <= 100 ? ptr->fanspeed_Overide : 0;
    log_message(LOG_INFO,"Hysteresis set to %d",hysteresis);
    log_message(LOG_INFO,"Fan Speeds set to %d%% %d%% %d%%",fanstage[0],fanstage[1],fanstage[2]);
    log_message(LOG_INFO,"Fan Temps set to %d %d %d",threshold[0],threshold[1],threshold[2]);
    log_message(LOG_INFO,"Fan Mode [ %s ] ", RUN_STATE_STR[runstate]);
    log_message(LOG_INFO,"Fan Speed Override %3d ", fanstage[3]);
    log_message(LOG_INFO,"Target Temperature %d ", threshold[3]);   
}

void Set_FanSpeed(uint8_t fan_speed)
{
    static int file_i2c = 0;
    static uint8_t speed = 0;
	if (file_i2c == 0)
    {
        char *filename = (char*)"/dev/i2c-1";
        if ((file_i2c = open(filename, O_RDWR)) < 0)
        {
            log_message(LOG_CRITICAL,"Failed to open the i2c bus");
            return;
        }
        int addr = 0x1a;
        if (ioctl(file_i2c, I2C_SLAVE, addr) < 0)
        {
            log_message(LOG_CRITICAL,"Failed to acquire bus access and/or talk to slave.");
            return;
        }
        log_message(LOG_INFO,"I2C Initialized");
    }
    if (fan_speed <= 100 && fan_speed != speed)
    {
        if (write(file_i2c, &fan_speed, 1) != 1)
        {
            log_message(LOG_CRITICAL,"Failed to write to the i2c bus.\n");
        }
        log_message(LOG_INFO, "Set fan to %d%%",fan_speed);
        speed = fan_speed;
        ptr->fanspeed = fan_speed;
    } else if (fan_speed == 0xFF)
    {
        close(file_i2c);
        log_message(LOG_INFO,"I2C Closed");
    }
}

void TMR_Get_temp(size_t timer_id, void *user_data)
{
    static uint32_t fdtemp = 0;
    uint32_t CPU_Temp = 0;
	static uint8_t fanspeed = 0;
    uint32_t property[10] =
    {
        0x00000000,
        0x00000000,
        0x00030006,
        0x00000008,
        0x00000004,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000
    };
    property[0] = 10 * sizeof(property[0]);
    if (user_data == NULL)
    {
        if (fdtemp == 0)
        {
            fdtemp = open("/dev/vcio", 0);
            if (fdtemp == -1)
            {
                log_message(LOG_CRITICAL, "Cannot get VideoCore I/O!");
                stop_timer(timer_id);
                log_message(LOG_CRITICAL, "Temperature can not be monitored!!");
            } else {
                log_message(LOG_INFO, "Successfully opened /dev/vcio for temperature sensor");
            }
        }
        if (ioctl(fdtemp, _IOWR(100, 0, char *), property) == -1)
        {
            log_message(LOG_CRITICAL, "Cannot get CPU Temp!");
            stop_timer(timer_id);
            log_message(LOG_CRITICAL, "Temperature can not be monitored!!");
        }
        CPU_Temp = property[6] / 1000;
        switch (runstate)
        {
            case 0: //AUTO
            switch (fanspeed)
            {
                case 0:
                if (CPU_Temp >= threshold[0]) fanspeed = 1;
                Set_FanSpeed(0);
                break;
                case 1:
                if (CPU_Temp >= threshold[1]) fanspeed = 2;
                if (CPU_Temp <= threshold[0] - hysteresis) fanspeed = 0;
                Set_FanSpeed(fanstage[0]);
                break;
                case 2:
                if (CPU_Temp >= threshold[2]) fanspeed = 3;
                if (CPU_Temp <= threshold[1] - hysteresis) fanspeed = 1;
                Set_FanSpeed(fanstage[1]);
                break;
                case 3:
                if (CPU_Temp <= threshold[2] - hysteresis) fanspeed = 2;
                Set_FanSpeed(fanstage[2]);
                break;
            }
            break;
            case 1: Set_FanSpeed(0); break; // OFF
            case 2: // MANUAL OVERRIDE
            Set_FanSpeed(fanstage[3]);
            break;
            case 3: // COOLDOWN
            if (CPU_Temp <= threshold[3])
            {
                log_message(LOG_INFO, "Cool down complete. switch to AUTO mode"); 
                Set_FanSpeed(0);
                runstate = 0;
            } else {
                Set_FanSpeed(fanstage[3]);
            }
            break;
        }
        ptr->temperature = CPU_Temp;
    } else {
        close(fdtemp);
        log_message(LOG_INFO, "Successfully closed temperature sensor");
    }
}

uint32_t monitor_device()
{
	struct gpioevent_request req;
	int fd;
	int ret;
	fd = open("/dev/gpiochip0", 0);
	if (fd == -1) {
		ret = -errno;
		goto exit_close_error;
	}
	req.lineoffset = 4;
	req.handleflags = GPIOHANDLE_REQUEST_INPUT;
	req.eventflags = GPIOEVENT_REQUEST_BOTH_EDGES;
	strcpy(req.consumer_label, "argonone-powerbutton");
	ret = ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &req);
	if (ret == -1) {
		ret = -errno;
		goto exit_close_error;
	}
	log_message(LOG_INFO, "Monitoring line 4 on /dev/gpiochip0");
    uint32_t Rtime = 0;
	while (1) {
		struct gpioevent_data event;
		ret = read(req.fd, &event, sizeof(event));
		if (ret == -1) {
			if (errno == -EAGAIN) {
				continue;
			} else {
				ret = -errno;
				break;
			}
		}
		if (ret != sizeof(event)) {
			ret = -EIO;
			break;
		}
		if (event.id == GPIOEVENT_EVENT_RISING_EDGE)
        	Rtime = event.timestamp / 1000000;
		if (event.id == GPIOEVENT_EVENT_FALLING_EDGE)
        {
			ret = (event.timestamp / 1000000) - Rtime;
			break;
		}
	}
exit_close_error:
	close(fd);
	return ret;
}

void daemonize(){
    int i,lfp;
    char str[10];
//  This is causing problems with systemd seems to be okay without
//    if(getppid() == 1)
//        return;
    i = fork();
    if(i < 0)
        exit(1);
    if(i > 0)
        exit(0);
    setsid();
    for(i = getdtablesize(); i >= 0; --i)
        close(i);
    i = open("/dev/null",O_RDWR);
    dup(i);
    dup(i);
    umask(0);
    chdir(RUNNING_DIR);
    lfp = open(LOCK_FILE,O_RDWR|O_CREAT,0640);
    if(lfp < 0)
    {
        log_message(LOG_FATAL,"Lock file can't be created");
        exit(1);
    }
    if(lockf(lfp,F_TLOCK,0) < 0)
    {
        log_message(LOG_FATAL,"Lock file cannot be locked");
        exit(1);
    }
    sprintf(str,"%d\n",getpid());
    if (write(lfp,str,strlen(str)) > 0) 
    {
        log_message(LOG_INFO,"Lock file created");
    } else {
        log_message(LOG_FATAL,"cannot write to lock file");
    }
    close(lfp);
    signal(SIGCHLD,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGHUP,signal_handler);
    signal(SIGTERM,signal_handler);
}

void gpioSetMode(unsigned gpio, unsigned mode)
{
   int reg, shift;
   log_message(LOG_INFO,"Set GPIO %d to mode %s", gpio, PI_MODE_STR[mode]);
   reg   =  gpio/10;
   shift = (gpio%10) * 3;
   gpioReg[reg] = (gpioReg[reg] & ~(7<<shift)) | (mode<<shift);
}

void gpioSetPullUpDown(unsigned gpio, unsigned pud)
{
   *(gpioReg + GPPUD) = pud;
   usleep(50);
   *(gpioReg + GPPUDCLK0 + (gpio>>5)) = 1<<(gpio&0x1F);
   usleep(50);
   *(gpioReg + GPPUD) = 0;
   *(gpioReg + GPPUDCLK0 + (gpio>>5)) = 0;
   log_message(LOG_INFO,"Set GPIO %d pull up/down to %s", gpio, PI_PUD_STR[pud]);
}

int gpioInitialise(void)
{
   int fd;
   fd = open("/dev/gpiomem", O_RDWR | O_SYNC) ;
   if (fd < 0)
   {
        log_message(LOG_FATAL, "failed to open /dev/gpiomem\n");
        return -1;
   }
   gpioReg = (uint32_t *)mmap(NULL, 0xB4, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
   close(fd);
   if (gpioReg == MAP_FAILED)
   {
        log_message(LOG_FATAL, "Bad, mmap failed\n");
        return -1;
   }
   return 0;
}

int main(int argc,char **argv)
{
    // check for unclean exit
    if (getuid() != 0) {
        fprintf(stderr, "ERROR: Permissions error, must be run as root\n");
        exit(1);
    }
    if (access(LOCK_FILE, F_OK) != -1)
    {
        FILE* file = fopen (LOCK_FILE, "r");
        int d_pid = 0;
        fscanf (file, "%d", &d_pid);
        fclose (file);
        if (kill(d_pid, 0) == 0)
        {
          fprintf(stderr, "argononed ALREADY RUNNING\n");
          exit (1);
        }
        log_message(LOG_WARN, "Unclean exit detected");
        unlink (LOCK_FILE);
        shm_unlink(SHM_FILE);
        log_message(LOG_INFO, "Clean up complete");
    }
    log_message(LOG_INFO,"Startup ArgonOne Daemon ver %s", VERSION);
    log_message(LOG_INFO,"Loading Configuration");
    Read_config();
    if (gpioInitialise() < 0)
    {
        log_message(LOG_FATAL,"GPIO initialise failed");
        return 1;
    }
    log_message(LOG_INFO,"GPIO initialized");
    
   struct identapi_struct Pirev;
   Pirev.RAW = IDENTAPI_GET_Revision();
   if (Pirev.RAW == 1)
   {
      log_message(LOG_FATAL,"Unable to read valid revision code");
      return 1;
   } else {

      float frev = 1.0 + (Pirev.REVISION / 10.0);
      char memstr[11];
      if (IDENTAPI_GET_int(Pirev, IDENTAPI_MEM) > 512) sprintf(memstr,"%dGB",IDENTAPI_GET_int(Pirev, IDENTAPI_MEM) / 1024);
      else sprintf(memstr,"%dMB",IDENTAPI_GET_int(Pirev, IDENTAPI_MEM));
      log_message(LOG_INFO, "RPI MODEL %s %s rev %1.1f", IDENTAPI_GET_str(Pirev, IDENTAPI_TYPE), memstr, frev);
   }
    daemonize();
    initialize_timers();
    Set_FanSpeed(0);
    log_message(LOG_INFO,"Now running as a daemon");
    size_t timer1 __attribute__((unused)) = start_timer_long(2, TMR_Get_temp,TIMER_PERIODIC,NULL);
    log_message(LOG_INFO, "Begin Initalizing shared memory");
	int shm_fd =  shm_open(SHM_FILE, O_CREAT | O_RDWR, 0666);
	ftruncate(shm_fd, SHM_SIZE);
    ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) {
		log_message(LOG_FATAL, "Shared memory map error");
		return 1;
	}
    memcpy(ptr->config.fanstages, &fanstage, sizeof(fanstage));
    memcpy(ptr->config.thresholds, &threshold, sizeof(threshold));
    ptr->config.hysteresis = hysteresis;
    gpioSetMode(4, PI_INPUT);
    gpioSetPullUpDown(4, PI_PUD_DOWN);
    log_message(LOG_INFO,"Now waitting for button press");
    uint32_t count = 0;
    do
    {
        count = monitor_device();
        log_message(LOG_DEBUG, "Pulse received %dms", count);
        if ((count >= 19 && count <= 21) || (count >= 39 && count <= 41)) break;
        else log_message(LOG_ERROR, "Unrecognized pulse width received");
    } while (1);
    cleanup();
    if (count >= 19 && count <= 21)
    {
        log_message(LOG_DEBUG, "EXEC REBOOT");
        sync();
        system("/sbin/reboot");
    }
    if (count >= 39 && count <= 41)
    {
        log_message(LOG_DEBUG, "EXEC SHUTDOWN");
        sync();
        system("/sbin/poweroff");
    }
    log_message(LOG_INFO,"Daemon Exiting");
    return 0;
}