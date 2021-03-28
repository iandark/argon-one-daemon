# Shared Memory

Details on the shared memory interface.

bytes 1 and 2 : are status bytes for fan speed and temperature  
bytes 3 - 9 : are configuration of set points same format as Overlay  
byte 10 : fan mode this will be used for switching how the fan will behave.  
> 0 - **AUTO** this is default the fan will follow the set points.  
> 1 - **OFF** the fan will not turn on or follow the set points.  
> 2 - **MANUAL** The fan will now follow the *fanspeed_Override* byte  
> 3 - **COOLDOWN** the fan will run at the *fanspeed_Override* byte  setting until the temperature stored in the *temperature_target* byte is reached. The fan will then switch to **AUTO**  

byte 11 : Temperature target this is used with special fan modes  
byte 12 : Fan speed override this is used with special fan modes  
byte 13 : Status byte  
> The shared memory will be validated before it's read. as such this byte will return a status.  
> 0 : OK  
> 1 : Validating  
> 2 : ERROR  

## Valid Ranges

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

*\**     Hysteresis can be as high as 10 if the thresholds are 11 degrees apart  
*\*\**   Fan Speed Override can be set by the daemon to 0 this is allowed  
*\*\*\** Undefined

---

## Version 0.3.x Improvements

This branch of code is meant to clean up and improve the 0.2.x branch.  This will fix compiler warnings and improve the shared memory interface.  Goals of this branch will be.

- fix compiler warnings as much as possible
- expand and finish the shared memory interface
- makefile and build changes for enabling or disabling features
- package system for embedded system

## Shared Memory [Expanded]

The goal of shared memory was to have control of the daemon from user space.  This version will expand and fix this interface.  Currently the interface requires the client to send a signal to the daemon to execute the request this requires the client to have root privileges.  This is taken care of with use of the sticky bit.  This version will expand the data bytes available in shared memory so that there is no longer overlap.  Meaning that a request message will not be over written by the daemon and that the daemon will have dedicated response bytes.  Status bytes will be expanded to include the following values "Current Fan Status, Current Temperature, Current Fan Mode, Current Set Points, Request Status" These values are all present in the current memory map however these values are also use as part of the request.  In the new version the current values will be readonly meaning that the request poller of the daemon will overwrite them and they will be ignored.

The on change to highlight of others mentioned is *Request Poller* This will be a timer running in the daemon to verify and read requests **without** the need of sending a signal to the daemon to execute the request.  A request will be execute when the control byte is set by the client and the daemon will update it's request status as required.  An important note is that the control byte will be overwritten by the daemon to clear the request. The only time the daemon will accept a request is when it's status byte is set to waiting.

The flow of requesting an action from the daemon will be as follows.

- **C** build request
- **C** set control byte to REQ_RDY *Request Ready*
- **D** set status byte to REQ_PEND *Request Pending*
- **D** copy request message out of shared memory and reset message
- **D** verify request data
- **D** ON ERROR in data set control byte to REQ_ERR and exit loop
- **D** Apply request
- **D** set status byte to REQ_WAIT

The valid values of the control and status bytes are

- REQ_WAIT - Waiting for request
- REQ_RDY - Request is ready for processing
- REQ_PEND - Request pending
- REQ_ERR - Error in last Request
- REQ_SYNC - Request Status to sync
- REQ_CLR - Clear request
- REQ_RST - Request Daemon to reset
- REQ_HOLD - Hold Requests
- REQ_OFF - Request Daemon to shutdown
- REQ_SIG - Request Commit Signal

## Types of requests

The request flags are not currently used however are defined. The flags are

- REQ_FLAG_MODE   0x01 : Request mode change
- REQ_FLAG_CONF   0x02 : Request Config change
- REQ_FLAG_CMD    0x04 : Request Command

These flags are used to set what type of request is being set.  It is possible for a request to use multiple flags the default request is equal to all flags being set.  As of ver 0.3.0 request flags are ignored.

## New changes to Shared Memory

Shared memory IPC has been expanded beyond the above mentioned limits and the interface is improving and expanding. New fields are being added to facilitate proper message areas better, daemon status reporting, and statistical tracking.
