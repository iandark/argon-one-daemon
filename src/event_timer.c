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

#include <stdint.h>
#include <string.h>
#include <sys/timerfd.h>
#include <pthread.h>
#include <poll.h>
#include <stdio.h>
#include <unistd.h>

#include "event_timer.h"

#define MAX_TIMER_COUNT 100

struct tmr_table
{
    int                 fd;
    time_handler        callback;
    void *              user_data;
    unsigned int        interval;
    tmr_types           type;
    struct tmr_table * next;
};

static void * _timer_thread();
static pthread_t tmr_pthread;
static struct tmr_table *tmr_table_pointer = NULL;

int initialize_timers()
{
    if(pthread_create(&tmr_pthread, NULL, _timer_thread, NULL))
    {
        /*Thread creation failed*/
        return 0;
    }

    return 1;
}

size_t start_timer_long (unsigned int interval, time_handler handler, tmr_types type, void * user_data)
{
    struct tmr_table * new_node = NULL;
    struct itimerspec new_value;

    new_node = (struct tmr_table *)malloc(sizeof(struct tmr_table));

    if(new_node == NULL) return 0;

    new_node->callback  = handler;
    new_node->user_data = user_data;
    new_node->interval  = interval;
    new_node->type      = type;

    new_node->fd = timerfd_create(CLOCK_REALTIME, 0);

    if (new_node->fd == -1)
    {
        free(new_node);
        return 0;
    }
   
    new_value.it_value.tv_sec = interval;
    new_value.it_value.tv_nsec = 0;

    if (type == TIMER_PERIODIC)
    {
      new_value.it_interval.tv_sec = interval;
    }
    else
    {
      new_value.it_interval.tv_sec = 0;
    }

    new_value.it_interval.tv_nsec= 0;

    timerfd_settime(new_node->fd, 0, &new_value, NULL);

    /*Inserting the timer node into the list*/
    new_node->next = tmr_table_pointer;
    tmr_table_pointer = new_node;

    return (size_t)new_node;
}

size_t start_timer(unsigned int interval, time_handler handler, tmr_types type, void * user_data)
{
    struct tmr_table * new_node = NULL;
    struct itimerspec new_value;

    new_node = (struct tmr_table *)malloc(sizeof(struct tmr_table));

    if(new_node == NULL) return 0;

    new_node->callback  = handler;
    new_node->user_data = user_data;
    new_node->interval  = interval;
    new_node->type      = type;

    new_node->fd = timerfd_create(CLOCK_REALTIME, 0);

    if (new_node->fd == -1)
    {
        free(new_node);
        return 0;
    }
   
    new_value.it_value.tv_sec = 0;
    new_value.it_value.tv_nsec = interval * 1000000;

    if (type == TIMER_PERIODIC)
    {
      new_value.it_interval.tv_nsec = interval * 1000000;
    }
    else
    {
      new_value.it_interval.tv_nsec = 0;
    }

    new_value.it_interval.tv_sec= 0;

    timerfd_settime(new_node->fd, 0, &new_value, NULL);

    /*Inserting the timer node into the list*/
    new_node->next = tmr_table_pointer;
    tmr_table_pointer = new_node;

    return (size_t)new_node;
}

void stop_timer(size_t timer_id)
{
    struct tmr_table * tmp = NULL;
    struct tmr_table * node = (struct tmr_table *)timer_id;

    if (node == NULL) return;

    close(node->fd);

    if(node == tmr_table_pointer)
    {
        tmr_table_pointer = tmr_table_pointer->next;
    } else {

        tmp = tmr_table_pointer;

        while(tmp && tmp->next != node) tmp = tmp->next;

        if(tmp)
        {
            tmp->next = tmp->next->next;
        }
    }
    if(node) free(node);
}

void close_timers()
{
    while(tmr_table_pointer) stop_timer((size_t)tmr_table_pointer);

    pthread_cancel(tmr_pthread);
    pthread_join(tmr_pthread, NULL);
}

struct tmr_table * _get_timer_from_fd(int fd)
{
    struct tmr_table * tmp = tmr_table_pointer;
    
    while(tmp)
    {
        if(tmp->fd == fd) return tmp;

        tmp = tmp->next;
    }
    return NULL;
}

void * _timer_thread()
{
    struct pollfd ufds[MAX_TIMER_COUNT] = {{0}};
    int iMaxCount = 0;
    struct tmr_table * tmp = NULL;
    int read_fds = 0, i, s;
    uint64_t exp;

    while(1)
    {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        iMaxCount = 0;
        tmp = tmr_table_pointer;

        memset(ufds, 0, sizeof(struct pollfd)*MAX_TIMER_COUNT);
        while(tmp)
        {
            ufds[iMaxCount].fd = tmp->fd;
            ufds[iMaxCount].events = POLLIN;
            iMaxCount++;

            tmp = tmp->next;
        }
        read_fds = poll(ufds, iMaxCount, 100);

        if (read_fds <= 0) continue;

        for (i = 0; i < iMaxCount; i++)
        {
            if (ufds[i].revents & POLLIN)
            {
                s = read(ufds[i].fd, &exp, sizeof(uint64_t));

                if (s != sizeof(uint64_t)) continue;

                tmp = _get_timer_from_fd(ufds[i].fd);

                if(tmp && tmp->callback) tmp->callback((size_t)tmp, tmp->user_data);
            }
        }
    }

    return NULL;
}
