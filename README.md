# Argon One Daemon

A replacement daemon for the Argon One Raspberry Pi case.

## How To Install

I've tried to make the installer as simple as possible. After cloning this repo simply run ```make all && sudo make install``` and that's it your argonone case is now up and running!

## Configuration

Configuration is all done in the **/boot/config.txt** look for this line ```dtoverlay=argonone``` The parameters are simple.
* **fantemp[0-2]** - Sets the temperatures at which the fan will spin up
* **fanspeed[0-2]** - Sets the speed at which the fan will spin 
* **hysteresis** - Sets the hysteresis 

The default values are the same as the OEM at 55℃ the fan will start at 10%, at 60℃ the speed will increase to 55% and finally after 65℃ the fan will spin at 100%.  The default hysteresis is 3℃

## Why make this?

Simply put I didn't like the OEM software.  It works sure but it uses Python and needs to install a bunch of dependencies.  This makes it's foot print on your system much bigger than it needs to be.  My daemon runs with minimal requirements, all of them are included in this Repo.

## Ubuntu or systems where /boot is different  

To install on a system like Ubuntu where /boot doesn't point to the boot partition.  
Run as **root** or ```make all && sudo make BOOTLOC=/boot/firmware install```   
***The installer will look to the default /boot unless BOOTLOC is set*** 

## Logging Options

The default build will generate a very detailed logs if you want less logging then add  
```make LOGLEVEL=[0-6]```  
The log levels go in this order: FATAL, CRITICAL, ERROR, WARNING, INFO, DEBUG. A value of 0 disables logging.

# The Argon One CLI tool  

This is the new command line tool that lets you change setting on the fly. It communicates with shared memory of the daemon, so the daemon must be running for this tool to be of use. It also introduced new modes for the daemon such as Cool Down and Manual control over the fan. 

### Cool Down Mode

In cool down mode the fan has a set temperature you want to reach before switching back to automatic control.  This is all set as follows   ```argonone-cli --cooldown <TEMP> [--fan <SPEED>]```  
***NOTE***: *The speed is optional and the default is 10% it's also import to note that if the temperature continues to climb the schedules set for the fan are ignored.*  

### Manual Mode  

As the name implies your in control over the fan the schedules are ignored.  To access this as follows ```argonone-cli --manual [--fan <SPEED>]```  
***NOTE***: *The fan speed is optional and if not set the fans speed is left alone.*

### Auto Mode

This is the default mode the daemon always starts in this mode and will follow the schedules in the setting.  If you want to change to automatic you do so as follows ```argonone-cli --auto```

### Off Mode

Yes an off switch, maybe you want to do something and you need to be sure the fan doesn't turn on and spoil it.  You can turn off the fan as follows ```argonone-cli --off``` 
***NOTE***: *When the fan is off nothing but turning to a different mode will turn it back on*

## Setting setpoints

Want to adjust the when the fan comes on, maybe it's not staying on long enough you can change all set points in the schedules from the command line **without** rebooting.  the values are fan[0-2] temp[0-2] and hysteresis.  It's important when changing these values that you remember that the daemon will reject bad values and/or change them to something else.  It's also important to commit the changes you make otherwise they won't do anything.  The value rules are simple each stage must to greater than the one before it and there are minimum and max values.  
For temperature the minimum value is 30° the maximum is currently undefined.  
For the fan the minimum speed is 10% and the maximum is 100%.  
For Hysteresis the minimum is 0° and the maximum is 10°  

You can set your values like in this example.  
```argonone-cli --fan0 25 --temp0 50 --hysteresis 10 --commit```   
**OR**  
```
argonone-cli --fan0 25
argonone-cli --temp0 50
argonone-cli --hysteresis 10
argonone-cli --commit
```
The changes don't have to made in one shot but you **MUST** commit them for them to take effect.

***IMPORTANT*** *There is no feedback from the daemon if the changes are applied successfully.  You can check the logs.  This feature is planned but not yet implemented*
