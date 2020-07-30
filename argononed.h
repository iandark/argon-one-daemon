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
/*  OVERLAY VERSION 1 Data
struct DTBO_Config_OLD {
    uint8_t fanstages[3];
    uint8_t thresholds[3];
    uint8_t hysteresis;
};
*/
struct DTBO_Config {
    uint8_t fanstages[3];
    uint8_t thresholds[3];
    uint8_t hysteresis;
};

struct SHM_Data {               //  DAEMON  |   CLIENT
    uint8_t fanspeed;           //      WO  |   RO
    uint8_t temperature;        //      WO  |   RO
    struct DTBO_Config config;  //      RW  |   RW
    uint8_t fanmode;            //      RW  |   RW
    uint8_t temperature_target; //      RW  |   RW
    uint8_t fanspeed_Overide;   //      RO  |   RW
    uint8_t status;             //      WO  |   RO
}; // current size - 13 bytes

void TMR_Get_temp(size_t timer_id, void *user_data);
void Set_FanSpeed(uint8_t fan_speed);
void reload_config_from_shm();

#endif