# ********************************************************************
# Argonone Daemon Makefile
# ********************************************************************
CC      = gcc
RM      = rm -v
DTC     = dtc -@ -I dts -O dtb -o
CFLAGS  = -Wall -s -O3
LFLAGS  = -lpthread
OBJ     = argononed.o event_timer.o
BINAME1 = argononed
BINAME2 = argonone-shutdown
OVERLAY = argonone.dtbo
GCCVER  = $(shell expr `gcc -dumpversion | cut -f1 -d.` \>= 10)
USERID	= $(shell id -u)

ifeq ($(GCCVER), 1)
	CFLAGs  += -fanalyzer
endif

ifeq (install,$(findstring install, $(MAKECMDGOALS)))
ifneq ($(USERID), 0)
$(error "(Un)Installing requires elevated privileges")
endif
endif

.DEFAULT_GOAL := all

%.o: %.c
	@echo "Compile $<"
	@$(CC) -c -o $@ $< $(CFLAGS)

$(BINAME1): $(OBJ)
	@echo "Build $(BINAME1)"
	$(CC) -o $(BINAME1) $^ $(CFLAGS) $(LFLAGS)

$(BINAME2): argonone-shutdown.c
	@echo "Build $(BINAME2)"
	$(CC) -o $(BINAME2) $^ $(CFLAGS)

$(OVERLAY): argonone.dts
	@echo "Build $@"
	$(DTC) $@ $<

.PHONY: overlay
overlay: $(OVERLAY)
	@echo "MAKE: Overlay"

.PHONY: daemon
daemon: $(BINAME1) $(BINAME2)
	@echo "MAKE: Daemon"

.PHONY: all
all: daemon overlay
	@echo "Make: Complete"

test:
	@echo "Current user id : $(USERID)"


.PHONY: install-daemon
install-daemon:
	@echo -n "Installing daemon "
	@install $(BINAME1) /usr/bin/$(BINAME1) 2>/dev/null && echo "Successful" || { echo "Failed"; true; }

.PHONY: install-overlay
install-overlay:
	@echo -n "Installing overlay "
	@install argonone.dtbo /boot/overlays/argonone.dtbo 2>/dev/null && echo "Successful" || { echo "Failed"; }
	@bash setup-overlay.sh

.PHONY: install-service
install-service:
	@echo "Installing services "
	@echo -n "argononed.service ... "
	@install -m 644 argononed.service /etc/systemd/system/argononed.service 2>/dev/null && echo "Successful" || { echo "Failed"; true; }
	@echo -n "argonone-shutdown ... "
	@install argonone-shutdown /lib/systemd/system-shutdown/argonone-shutdown 2>/dev/null && echo "Successful" || { echo "Failed"; true; }
	@echo "Refresh services list"
	@systemctl daemon-reload
	@echo -n "Enable Service "
	@systemctl enable argononed 2>/dev/null && echo "Successful" || { echo "Failed"; }
	@echo -n "Starting Service "
	@systemctl start argononed 2>/dev/null && echo "Successful" || { echo "Failed"; }

.PHONY: install
install: install-daemon install-service install-overlay
	@echo "Install Complete"

.PHONY: uninstall
uninstall:
	@echo -n "Stop Service ... "
	@systemctl stop argononed 2>/dev/null && echo "Successful" || { echo "Failed"; }
	@echo -n "Disable Service ... "
	@systemctl disable argononed 2>/dev/null && echo "Successful" || { echo "Failed"; }
	@echo -n "Erase Service ... "
	@$(RM) /etc/systemd/system/argononed.service 2>/dev/null&& echo "Successful" || { echo "Failed"; true; }
	@echo -n "Remove overlay ... "
	@$(RM) /boot/overlays/argonone.dtbo 2>/dev/null && echo "Successful" || { echo "Failed"; }
	@echo -n "Erase argonone-shutdown ... "
	@$(RM) /lib/systemd/system-shutdown/argonone-shutdown 2>/dev/null && echo "Successful" || { echo "Failed"; true; }
	@echo -n "Remove daemon ... "
	@$(RM) /usr/bin/argononed 2>/dev/null&& echo "Successful" || { echo "Failed"; true; }
	@echo "Remove dtoverlay=argonone from /boot/config.txt"
	@cp /boot/config.txt /boot/config.argoneone.backup
	@sed -i '/dtoverlay=argonone/d' /boot/config.txt
	@echo "Uninstall Complete"

.PHONY: clean
clean:
	-@$(RM) *.o 2>/dev/null || true
	-@$(RM) argonone.dtbo 2>/dev/null || true
	-@$(RM) $(BINAME) 2>/dev/null || true
