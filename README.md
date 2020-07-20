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
