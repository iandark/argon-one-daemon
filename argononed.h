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

#ifndef ARGONONED_H
#define ARGONONED_H

#define RUNNING_DIR "/tmp"
#define LOG_FILE "/var/log/argononed.log"
#define LOCK_FILE "/run/argononed.pid"
#define SHM_FILE "argonone"
#define SHM_SIZE 512

#define PI_INPUT  0
#define PI_OUTPUT 1
#define GPPUD     37
#define GPPUDCLK0 38
#define PI_PUD_OFF  0
#define PI_PUD_DOWN 1
#define PI_PUD_UP   2

struct DTBO_Config {
    uint8_t fanstages[3];
    uint8_t thresholds[3];
    uint8_t hysteresis;
};

#define REQ_WAIT 0              // Waiting for request 
#define REQ_RDY  1              // Request is ready for processing
#define REQ_PEND 2              // Request pending
#define REQ_ERR  3              // Error in last Request
#define REQ_SYNC 4              // Request Status to sync
#define REQ_CLR  5              // Clear request
#define REQ_RST  6              // Request Daemon to reset 
#define REQ_HOLD 7              // Hold Requests
#define REQ_OFF  8              // Request Daemon to shutdown
#define REQ_SIG  9              // Request Commit Signal

#define REQ_FLAG_MODE   0x01    // Request mode change
#define REQ_FLAG_CONF   0x02    // Request Config change
#define REQ_FLAG_CMD    0x04    // Request Command

struct SHM_Data {               //  DAEMON  |   CLIENT
    uint8_t fanspeed;           //      WO  |   RO
    uint8_t temperature;        //      WO  |   RO
    struct DTBO_Config config;  //      RW  |   RW
    uint8_t fanmode;            //      RW  |   RW
    uint8_t temperature_target; //      RW  |   RW
    uint8_t fanspeed_Overide;   //      RO  |   RW
    uint8_t status;             //      RW  |   RW
    uint8_t req_flags;          //      RW  |   WO
}; // current size - 14 bytes

void TMR_Get_temp(size_t timer_id, void *user_data);
void Set_FanSpeed(uint8_t fan_speed);
int reload_config_from_shm();

#endif