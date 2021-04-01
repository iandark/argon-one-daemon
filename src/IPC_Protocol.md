# Introduction

This is the documentation of the protocol used to communicate with the daemon via the shared memory.

## Memory Map

The DTBO_Config data structure or DeviceTree Configuration matches the format of the data format used by the overlay.  The size is 7 bytes.

```C
struct DTBO_Config {
    uint8_t fanstages[3];
    uint8_t thresholds[3];
    uint8_t hysteresis;
};
```

The SHM_DAEMON_STATS data structure is the Daemon Statistics, The size is 5 bytes and these bytes are **READONLY**.

```C
struct SHM_DAEMON_STATS {
    uint8_t max_temperature;
    uint8_t min_temperature;
    uint8_t EF_Warning;
    uint8_t EF_Error;
    uint8_t EF_Critical;
};
```

The SHM_REQ_MSG data structure or Request Message. The size is 12 bytes.  This structure is used to send messages to the daemon.

```C
struct SHM_REQ_MSG {
    uint8_t req_flags;
    struct DTBO_Config Schedules;
    uint8_t fanmode;
    uint8_t temperature_target;
    uint8_t fanspeed_Overide;
    uint8_t status;
};
```

The SHM_Data structure is the full memory map. This uses 31 bytes.

```C
struct SHM_Data {
    uint8_t fanspeed;
    uint8_t temperature;
    struct DTBO_Config config;
    uint8_t fanmode;
    uint8_t temperature_target;
    uint8_t fanspeed_Overide;
    uint8_t status;
    uint8_t req_flags;
    struct SHM_DAEMON_STATS stat;
    struct SHM_REQ_MSG msg;
};
```

The fields are all 1 byte wide

| Byte(s)   | Name                  | Minimum   | Maximum   |
| :-------: | :-------------------- | :-------- | :-------- |
| 1         | Current Fan Speed     | 0         | 100       |
| 2         | Current Temperature   | 0         | 85        |
| 3 - 5     | Fan Speed 1 - 3       | 0         | 100       |
| 6 - 8     | Threshold 1 - 3       | 30        | 85        |
| 9         | Hysteresis            | 0         | 5(10)*\** |
| 10        | Fan Mode              | 0         | 3         |
| 11        | Temperature Target    | 30        | 85        |
| 12        | Fan Speed Override    | 10 *\*\** | 100       |
| 13        | Status of Request     | 0         | 2         |  
| 14        | Request Flags         | 0         | -- *\*\*\**|
| **15 - 19**| **Daemon Statistic** |           |           |
| **20 - 31**| **Request Message**  |           |           |
| **32 - 43**| **Request Message**  | **RESERVED**          |

*As of version 0.3.1*  
*Bytes 32 - 43 are **RESERVED** for a second message channel TBD*

*\**     Hysteresis can be as high as 10 if the thresholds are 11 degrees apart  
*\*\**   Fan Speed Override can be set by the daemon to 0 this is allowed  
*\*\*\** Undefined
