## Shared Memory

Details on the shared memory interface.

bytes 1 and 2 : are status bytes for fan speed and temperature  
bytes 3 - 9 : are configuration of set points same format as Overlay  
byte 10 : fan mode this will be used for switching how the fan will behave.  
>   0 - **AUTO** this is default the fan will follow the set points.   
>   1 - **OFF** the fan will not turn on or follow the set points.  
>   2 - **MANUAL** The fan will now follow the *fanspeed_Override* byte  
>   3 - **COOLDOWN** the fan will run at the *fanspeed_Override* byte  setting until the temperature stored in the *temperature_target* byte is reached. The fan will then switch to **AUTO**  

byte 11 : Temperature target this is used with special fan modes  
byte 12 : Fan speed override this is used with special fan modes  
byte 13 : Status byte  
>   The shared memory will be validated before it's read. as such this byte will return a status.    
>  0 : OK  
>  1 : Validating  
>  2 : ERROR  

### Valid Ranges 

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
*\**    Hysteresis can be as high as 10 if the thresholds are 11 degrees apart  
*\*\**  Fan Speed Override can be set by the daemon to 0 this is allowed