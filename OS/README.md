# OS folders

This section is used for customized procedures required for an OS installer. The configure script looks for ``OS/$(DISTRO)/OS.conf`` and the main makefile will include the ``OS/$(DISTRO)/makefile.in``  

The makefile can add to but not override the targets `install` or `uninstall`  

Any support files required for the OS must be in it's directory

This system is limited in that the OS' `/etc/os-release` is used to identify it using the ID variable.

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
