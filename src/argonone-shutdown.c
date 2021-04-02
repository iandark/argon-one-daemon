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
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc == 1) return 0;
    if (strstr(" poweroff halt ", argv[1]) != NULL) 
    {
        int file_i2c = 0;
        char cmd_shutdown = 0x00;  // Turn off Fan 
        char *filename = (char*)"/dev/i2c-1";
        if ((file_i2c = open(filename, O_RDWR)) < 0)
        {
            return -1;
        }
        int addr = 0x1a;
        if (ioctl(file_i2c, I2C_SLAVE, addr) < 0)
        {
            return -1;
        }
        if (write(file_i2c, &cmd_shutdown, 1) != 1)
        {
            return -1;
        }
        cmd_shutdown = 0xff;        // Shutdown Command
        if (write(file_i2c, &cmd_shutdown, 1) != 1)
        {
            return -1;
        }
        close(file_i2c);
    }
    return 0;
}
