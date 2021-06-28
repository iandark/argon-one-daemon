# How to OpenWRT install

## Install steps

```text
opkg install gcc make git git-http apk alpine-repositories dtc bash kmod-i2c-core kmod-i2c-bcm2835 kmod-i2c-algo-bit kmod-i2c-gpio
apk --allow-untrusted update
apk --allow-untrusted add argp-standalone
git clone https://gitlab.com/DarkElvenAngel/argononed.git
cd argononed
git checkout 0.3.x
TARGET_DISTRO=openwrt ./package.sh 
```

### The expected output

```text
    ___                                                __
   /   |  _________ _____  ____  ____  ____  ___  ____/ /
  / /| | / ___/ __ `/ __ \/ __ \/ __ \/ __ \/ _ \/ __  / 
 / ___ |/ /  / /_/ / /_/ / / / / /_/ / / / /  __/ /_/ /  
/_/  |_/_/   \__, /\____/_/ /_/\____/_/ /_/\___/\__,_/   
            /____/                                       
                                                PACKAGER 
_________________________________________________________
ARGON ONE DAEMON CONFIGURING ...
Distro check [openwrt] : EXPERIMENTAL
SYSTEM CHECK
gcc : OK
dtc : OK
make : OK
I2C Bus check : NOT ENABLED
CHECKING OPTIONAL SYSTEMS
bash-autocomplete : NOT INSTALLED
logrotate : INSTALLED
Dependency Check : Successful
INFO:  Preparing build environment ... OK
INFO:  Building Source Files ... OK
INFO:  Checking files ... OK
INFO:  Building Installer ... OK
INFO:  Packing files ... OK
INFO:  Verify package ... OK
INFO:  Package build/openwrt.pkg.sh is complete 
```

Execute `./build/openwrt.pkg.sh`

```text
INFO:  ArgonOne Daemon self extracting installer
INFO:  Checking installer ... OK
INFO:  Starting installer
INFO:  Verify package list ... FOUND!
INFO:  Extracting files ... OK
INFO:  Verify contents ...OK
INFO:  Installing
Search config.txt for overlay ... NOT FOUND
Insert overlay into /boot/config.txt ... DONE
reboot required
```

### Reboot the system and now everything should be working

This can be confirmed with `argonone-cli --decode`

```text
>> DECODEING MEMORY <<
Fan Status OFF Speed 0%
System Temperature 39°
Hysteresis set to 3°
Fan Speeds set to 10% 55% 100%
Fan Temps set to 55° 60° 65°
Fan Mode [ AUTO ] 
Fan Speed Override 0% 
Target Temperature 0°
Daemon Status : Waiting for request
Maximum Temperature : 39°
Minimum Temperature : 33°
Daemon Warnings : 0
Daemon Errors : 0
Daemon Critical Errors : 0
```

### Checking the logs

```text
Sat Jun 26 15:12:51 2021 [INFO] Startup ArgonOne Daemon ver 0.3.2
Sat Jun 26 15:12:51 2021 [INFO] Loading Configuration
Sat Jun 26 15:12:51 2021 [INFO] Reading values from device-tree
Sat Jun 26 15:12:51 2021 [INFO] Hysteresis set to 3
Sat Jun 26 15:12:51 2021 [INFO] Fan Speeds set to 10% 55% 100%
Sat Jun 26 15:12:51 2021 [INFO] Fan Temps set to 55 60 65
Sat Jun 26 15:12:51 2021 [INFO] GPIO initialized
Sat Jun 26 15:12:51 2021 [INFO] RPI MODEL 4B 4GB rev 1.1
Sat Jun 26 15:12:51 2021 [INFO] Lock file created
Sat Jun 26 15:12:51 2021 [INFO] Now running as a daemon
Sat Jun 26 15:12:51 2021 [INFO] Begin Initalizing shared memory
Sat Jun 26 15:12:51 2021 [INFO] I2C Initialized
Sat Jun 26 15:12:51 2021 [INFO] Set fan to 0%
Sat Jun 26 15:12:51 2021 [INFO] Set GPIO 4 to mode INPUT
Sat Jun 26 15:12:51 2021 [INFO] Set GPIO 4 pull up/down to DOWN
Sat Jun 26 15:12:51 2021 [INFO] Now waiting for button press
Sat Jun 26 15:12:51 2021 [INFO] Monitoring line 4 on /dev/gpiochip0
Sat Jun 26 15:12:53 2021 [INFO] Successfully opened /dev/vcio for temperature sensor
```

Everything looks okay!
