# OS folders

This section is used for customized procedures required for an OS installer. The configure script looks for ``OS/$(DISTRO)/OS.conf`` and the main makefile will include the ``OS/$(DISTRO)/makefile.in``  

The makefile can add to but not override the targets `install` or `uninstall`  

Any support files required for the OS must be in it's directory

This system is limited in that the OS' `/etc/os-release` is used to identify it using the ID variable.

## How to add support of an OS

Adding support for a new OS should not be a difficult task however some systems are easier than others.  With that in mind we can begin fire we need to know if the OS can be identified by configure.  Run this command and not the output on the target OS `awk -F"=" '$1=="ID"{print $2}' /etc/os-release` for example on Raspberry Pi OS this returns _raspbian_. Now that we know the correct name of our OS we create the folder in OS following our example this is OS/raspbian.  Now we need to configure for the OS so create OS.conf

```text
STATUS="EXPERIMENTAL"
INITSYS="SYSTEMD"
BOOTLOC="/boot"
```

For any new OS it's advisable to set status to **EXPERIMENTAL**, the init system used by out OS is next in the example this is systemd (_most common_) and finally the boot location.  This is where the boot partition is mounted.

If you need to have custom events happen in order to install then it's best to create makefile.in from there it's possible to define any changes needed how ever as noted it's not possible to override the standard _install_ or _uninstall_ but you can add steps.

## Variables must be set in the makefile.in

- **SERVICE_FILE** - Source service file
- **SERVICE_FILE_PERMISSIONS** - Target file permissions for service file
- **SERVICE_PATH** - Install path for service file *can also rename if required*
- **SHUTDOWN_FILE** - Source shutdown
- **SHUTDOWN_PATH** - Install path for shutdown
- **SERVICE_ENABLE** - Command to enable service
- **SERVICE_DISABLE** - Command to disable service
- **SERVICE_START** - Command to start service
- **SERVICE_STOP** - Command to stop service

## Variables must be set in the OS.conf

- **STATUS** - The current status of the OS' support [ OK | EXPERIMENTAL | PACK ]
- **INITSYS** - The type of init system used for services
- **I2CHELPER** - If the OS needs extra steps to enable I2C
- **BOOTLOC** - Location of where boot partition is mounted
- **PACKAGESYS** - Set to 1 if native install is not possible

## Add Package Support

Building an install package for an OS can be the only way to install for in some cases this can be due to a lack of compiler or other build system needs that simply do not exist for the OS in question.  Let's continue with the example from before of _raspbian_ if we wanted to block local building we could edit our OS.conf and add **PACKAGESYS=1** this would stop native install from working.  however we can leave this out since it's possible to do a native install. In order for the package system to know want to do we need to make a pkg_lst file and fill it with all the files we need.

```text
OS/raspbian/pkg_list
build/argononed
build/argonone-cli
build/argonone-shutdown
build/argonone.dtbo
OS/_common/argononed.service
OS/_common/setup-overlay.sh
```

Now that the packager know what to package up we have to tell it how to install it.  For this we need a pkg_install.sh script.

```sh
F_INSTALL()
{
    echo -n "INFO:  Verify package list ... "
    tail -n+${DATA_START} $0 | tar tzv OS/rasbian/pkg_list &>/dev/null && echo "FOUND!" || { echo "MISSING!"; exit 1; } 
    F_EXTRACT
    echo -n "INFO:  Verify contents ..."
    while read line; do
    # reading each line
    [[ -f $line ]] || { echo -e "ERR\nERROR:  ${line} File Not Found!"; exit 1; }
    done < OS/rasbian/pkg_list
    echo "OK"
    echo "INFO:  Installing"
    install build/argononed /usr/sbin/argononed 2>/dev/null || { echo "ERROR:  Cannot install argononed"; exit 1;}
    install -m 4755 build/argonone-cli /usr/bin/argonone-cli 2>/dev/null || { echo "ERROR:  Cannot install argonone-cli"; exit 1;}
    install build/argonone-shutdown /lib/systemd/system-shutdown/argonone-shutdown 2>/dev/null || { echo "ERROR:  Cannot install argonone-shutdown"; exit 1;}
    install OS/_common/argononed.service /etc/systemd/system/argononed.service || { echo "ERROR:  Cannot install argononed.service"; exit 1;}
    systemctl daemon-reload
    systemctl enable argononed
    install build/argonone.dtbo /boot/overlays
    sh OS/_common/setup-overlay.sh /boot/config.txt
    [[ -r /dev/i2c-1 ]] ||  echo "reboot required" && systemctl start argononed
}
```

WOW! lets break that down the function **F_INSTALL()** must exist and call **F_EXTRACT** verifying all the files were copied is alway good practice. Then we come to Installing just copy our unpacked file to where then need to go.  Then we no let the system know we added a new service and set it to run on the next boot.  Next we copy the device tree overlay to the boot partition.  The setup-overlay script can add this for us to the config.txt Optionally we can check if the i2c bus is available if it is we can start the daemon other wise we should tell the user to reboot **NEVER COMMAND THE SYSTEM TO REBOOT FROM THE SCRIPT** and that's it we can now build a package for _raspbian_.

### Final thoughts

If you write support for a new system and it works well then consider  submitting a pull request to help the project out.
