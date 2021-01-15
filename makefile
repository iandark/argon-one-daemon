# ********************************************************************
# Argonone Daemon Makefile
# ********************************************************************
CC      = gcc
RM      = rm -v
DTC     = dtc -@ -I dts -O dtb -o
BASH	= bash
INSTALL = install
CFLAGS  = -Wall -s -O3 -Wextra -Wpedantic
LFLAGS  = -lpthread -lrt
OBJ     = argononed.o event_timer.o
BINAME1 = argononed
BINAME2 = argonone-shutdown
BINAME3 = argonone-cli
OVERLAY = argonone.dtbo
GCCVER  = $(shell expr `gcc -dumpversion | cut -f1 -d.` \>= 10)
USERID	= $(shell id -u)
LOGLEVEL = 6

-include makefile.conf
ifndef BOOTLOC
BOOTLOC = /boot
endif
ifndef INITSYS
INITSYS = SYSTEMD
endif
ifndef I2CHELPER
I2CHELPER = 0
endif
ifndef AUTOCOMP
AUTOCOMP = 0
endif

ifeq ($(GCCVER), 1)
	CFLAGs  += -fanalyzer
endif

ifeq ($(INITSYS), SYSTEMD)
	SERVICE_FILE=argononed.service
	SERVICE_FILE_PERMISSIONS=644
	SERVICE_PATH=/etc/systemd/system/argononed.service
	SHUTDOWN_FILE=$(BINAME2)
	SHUTDOWN_PATH=/lib/systemd/system-shutdown/argonone-shutdown
	SERVICE_ENABLE=systemctl enable
	SERVICE_DISABLE=systemctl disable
	SERVICE_START=systemctl start argononed
	SERVICE_STOP=systemctl stop argononed
endif
-include OS/$(DISTRO)/makefile.in
ifndef SERVICE_FILE
$(error makefile configuration error!)
endif

ifeq (install,$(findstring install, $(MAKECMDGOALS)))
ifneq ($(USERID), 0)
$(error "(Un)Installing requires elevated privileges")
endif
endif

.DEFAULT_GOAL := all

%.o: %.c
	@echo "Compile $<"
	$(CC) -c -o $@ $< $(CFLAGS) -DLOG_LEVEL=$(LOGLEVEL) 

$(BINAME1): $(OBJ)
	@echo "Build $(BINAME1)"
	$(CC) -o $(BINAME1) $^ $(CFLAGS) $(LFLAGS)

$(BINAME2): argonone-shutdown.c
	@echo "Build $(BINAME2)"
	$(CC) -o $(BINAME2) $^ $(CFLAGS)

$(BINAME3): argonone-cli.c
	@echo "Build $(BINAME3)"
	$(CC) -o $(BINAME3) $^ $(CFLAGS) -DLOG_LEVEL=$(LOGLEVEL) -lrt

$(OVERLAY): argonone.dts
	@echo "Build $@"
	$(DTC) $@ $<

.PHONY: overlay
overlay: $(OVERLAY)
	@echo "MAKE: Overlay"

.PHONY: daemon
daemon: $(BINAME1) $(BINAME2)
	@echo "MAKE: Daemon"

.PHONY: cli
cli: $(BINAME3)
	@echo "MAKE: CLI"

.PHONY: all
all: daemon cli overlay
	@echo "MAKE: Complete"

.PHONY: install-daemon
install-daemon:
	@echo -n "Installing daemon "
	@$(INSTALL) $(BINAME1) /usr/bin/$(BINAME1) 2>/dev/null && echo "Successful" || { echo "Failed"; true; }

.PHONY: install-cli
install-cli:
	@echo -n "Installing CLI "
	@$(INSTALL) -m 4755 $(BINAME3) /usr/bin/$(BINAME3) 2>/dev/null && echo "Successful" || { echo "Failed"; true; }
ifeq ($(AUTOCOMP), 1)
	@echo -n "Installing CLI autocomplete for bash "
	@$(INSTALL) -m 755 argonone-cli-complete.bash /etc/bash_completion.d/argoneone-cli 2>/dev/null && echo "Successful" || { echo "Failed"; true; }
endif

.PHONY: install-overlay
install-overlay:
	@echo -n "Installing overlay "
	@$(INSTALL) argonone.dtbo $(BOOTLOC)/overlays/argonone.dtbo 2>/dev/null && echo "Successful" || { echo "Failed"; }
	@$(BASH) setup-overlay.sh $(BOOTLOC)/config.txt


.PHONY: install-service
install-service:
	@echo "Installing services "
	@echo -n "argononed.service ... "
	@$(INSTALL) -m $(SERVICE_FILE_PERMISSIONS) $(SERVICE_FILE) $(SERVICE_PATH) 2>/dev/null && echo "Successful" || { echo "Failed"; true; } 
	@echo -n "argonone-shutdown ... "
	@$(INSTALL) $(SHUTDOWN_FILE) $(SHUTDOWN_PATH) 2>/dev/null && echo "Successful" || { echo "Failed"; true; }
ifeq ($(INITSYS), SYSTEMD)
	@echo "Refresh services list"
	@systemctl daemon-reload
endif
	@echo -n "Enable Service "
	@$(SERVICE_ENABLE) argononed &>/dev/null && echo "Successful" || { echo "Failed"; }
	@echo -n "Starting Service "
	@timeout 5s $(SERVICE_START) &>/dev/null && echo "Successful" || { ( [ $$? -eq 124 ] && echo "Timeout" || echo "Failed" ) }

.PHONY: install
install:: install-daemon install-cli install-service install-overlay
	@echo "Install Complete"

.PHONY: uninstall
uninstall:
	@echo -n "Stop Service ... "
	@$(SERVICE_STOP) &>/dev/null && echo "Successful" || { echo "Failed"; }
	@echo -n "Disable Service ... "
	@$(SERVICE_DISABLE) &>/dev/null && echo "Successful" || { echo "Failed"; }
	@echo -n "Erase Service ... "
	@$(RM) $(SERVICE_PATH) 2>/dev/null && echo "Successful" || { echo "Failed"; true; }
ifeq ($(INITSYS), OPENRC)
	@echo -n "Erase Shutdown Service ... "
	@$(RM) $(SHUTDOWN_PATH) 2>/dev/null && echo "Successful" || { echo "Failed"; true; }
	@echo -n "Erase argonone-shutdown ... "
	@$(RM) /usr/bin/shutdown_argonone 2>/dev/null && echo "Successful" || { echo "Failed"; true; }
else
	@echo -n "Erase argonone-shutdown ... "
	@$(RM) $(SHUTDOWN_PATH) 2>/dev/null && echo "Successful" || { echo "Failed"; true; }
endif
	@echo -n "Remove overlay ... "
	@$(RM) $(BOOTLOC)/overlays/argonone.dtbo 2>/dev/null && echo "Successful" || { echo "Failed"; }
	@echo -n "Remove daemon ... "
	@$(RM) /usr/bin/argononed 2>/dev/null&& echo "Successful" || { echo "Failed"; true; }
	@echo -n "Remove cli-tool ... "
	@$(RM) /usr/bin/argonone-cli 2>/dev/null&& echo "Successful" || { echo "Failed"; true; }
	@echo -n "Remove autocomplete for cli ... "
	$(RM) /etc/bash_completion.d/argoneone-cli 2>/dev/null && echo "Successful" || { echo "Failed"; true; }
	@echo "Remove dtoverlay=argonone from $(BOOTLOC)/config.txt"
	@cp $(BOOTLOC)/config.txt $(BOOTLOC)/config.argoneone.backup
	@sed -i '/dtoverlay=argonone/d' $(BOOTLOC)/config.txt
	@echo "Uninstall Complete"

.PHONY: clean
clean:
	-@$(RM) *.o 2>/dev/null || true
	-@$(RM) argonone.dtbo 2>/dev/null || true
	-@$(RM) $(BINAME1) 2>/dev/null || true
	-@$(RM) $(BINAME2) 2>/dev/null || true
	-@$(RM) $(BINAME3) 2>/dev/null || true

.PHONY: mrproper
mrproper: clean
	-@$(RM) makefile.conf 2>/dev/null || true
