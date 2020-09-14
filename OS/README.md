# OS folders

This section is used for customised procedures required for an OS the main makefile will include the ``OS/$(DISTRO)/makefile.in``  

The makefile can add to but not override the targets `install` or `uninstall`  

It is also a good place to add any support files required for that OS.

This system is limited in that the OS' `/etc/os-release` is used to identify it using the ID variable.

## Critical Variable that should be set

-	**SERVICE_FILE** - Source service file
-	**SERVICE_FILE_PERMISSIONS** - Target file permissions for service file
-	**SERVICE_PATH** - Install path for service file *can also rename if required*
-	**SHUTDOWN_FILE** - Source shutdown
-	**SHUTDOWN_PATH** - Install path for shutdown 
-	**SERVICE_ENABLE** - Command to enable service
-	**SERVICE_DISABLE** - Command to disable service
-	**SERVICE_START** - Command to start service
-	**SERVICE_STOP** - Command to stop service 