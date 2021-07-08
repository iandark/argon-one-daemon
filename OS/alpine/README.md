# Alpine Linux

## Status

**INCOMPATIBLE** with Argon One all versions  
**EXPERIMENTAL** with Argon ARTIK hat

## Argon ARTIK hat

The Daemon is fully working with this setup the steps below document how to setup for install.

## Argon One Case

This product is unsupportable at this time.  There is no way known to detect a power off vs reboot on this OS without hacks.  Such changes to the system aren't recommended as they may provide challenges to undo or break future updates.

## Install Steps

***This isn't a guild to setup ALPINE LINUX***  
Setup the build environment install the required packages.  
```apk install gcc dtc git argp-standalone bash build-base linux-headers```  
clone the repo and follow the build steps.

### Case hack

**THIS IS NOT RECOMMEND BUT I'M SUPPLYING THIS SOLUTION TO USE AT YOUR OWN RISK** With the warning out of the way here is the hack.

Remove **/sbin/reboot** and replace it with this script

```sh
#!/bin/sh
touch /tmp/reboot
busybox reboot
```

You need to set the execution bit. Next copy the **OS/alpine/argononed.stop** script to **/etc/local.d**. Last you need to activate the script `rc-update add local`.  Now the hack is complete.
