F_INSTALL()
{
    echo -n "INFO:  Verify package list ... "
    tail -n+${DATA_START} $0 | tar tzv OS/openwrt/pkg_list &>/dev/null && echo "FOUND!" || { echo "MISSING!"; exit 1; } 
    F_EXTRACT
    echo -n "INFO:  Verify contents ..."
    while read line; do
    # reading each line
    [[ -f $line ]] || { echo -e "ERR\nERROR:  ${line} File Not Found!"; exit 1; }
    done < OS/openwrt/pkg_list
    echo "OK"
    echo "INFO:  Installing"
    cp build/argononed /usr/sbin/argononed 2>/dev/null || { echo "ERROR:  Cannot install argononed"; exit 1;}
    cp build/argonone-cli /usr/bin/argonone-cli 2>/dev/null || { echo "ERROR:  Cannot install argonone-cli"; exit 1;}
    cp build/argonone-shutdown /usr/sbin/argonone-shutdown 2>/dev/null || { echo "ERROR:  Cannot install argonone-shutdown"; exit 1;}
    cp OS/openwrt/argononed /etc/init.d/argononed || { echo "ERROR:  Cannot install argononed.service"; exit 1;}
    chmod +x /etc/init.d/argononed
    /etc/init.d/argononed enable
    cp build/argonone.dtbo /boot/overlays
    sh OS/_common/setup-overlay.sh /boot/config.txt
    [[ -r /dev/i2c-1 ]] ||  echo "reboot required" && /etc/init.d/argononed start
}