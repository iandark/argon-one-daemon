F_INSTALL()
{
    #DATA_START=$((`grep -an "^DATA_CONTENT$" $0 | cut -d: -f1` + 1))
    echo -n "INFO:  Verify package list ... "
    tail -n+${DATA_START} $0 | tar tzv OS/libreelec/pkg_list &>/dev/null && echo "FOUND!" || { echo "MISSING!"; exit 1; } 
    F_EXTRACT
    echo -n "INFO:  Verify contents ..."
    while read line; do
    # reading each line
    [[ -f $line ]] || { echo -e "ERR\nERROR:  ${line} File Not Found!"; exit 1; }
    done < OS/libreelec/pkg_list
    echo "OK"
    echo "INFO:  Installing"
    [[ -d /storage/sbin ]] || mkdir /storage/sbin
    install build/argononed /storage/sbin/argononed 2>/dev/null || { echo "ERROR:  Cannot install argononed"; exit 1;}
    install -m 4755 build/argonone-cli /storage/bin/argonone-cli 2>/dev/null || { echo "ERROR:  Cannot install argonone-cli"; exit 1;}
    install build/argonone-shutdown /storage/sbin/argonone-shutdown 2>/dev/null || { echo "ERROR:  Cannot install argonone-shutdown"; exit 1;}
    # Change path form /usr/sbin to /storage/sbin
    sed -i "s/usr/storage/g" OS/_common/argononed.service
    install OS/_common/argononed.service /storage/.config/system.d/argononed.service || { echo "ERROR:  Cannot install argononed.service"; exit 1;}
    systemctl daemon-reload
    systemctl enable argononed
    mount -o remount,rw /flash
    install build/argonone.dtbo /flash/overlays
    sh OS/_common//setup-overlay.sh /flash/config.txt
    mount -o remount,ro /flash
    if [[ -f /storage/.config/shutdown.sh ]]
    then
    {
        echo "/storage/sbin/argonone-shutdown \$1" >> /storage/.config/shutdown.sh
    } else {
        cat > /storage/.config/shutdown.sh << END_SCRIPT
#!/bin/bash
/storage/sbin/argonone-shutdown \$1
END_SCRIPT
        chmod +x /storage/.config/shutdown.sh
    }
    fi
    [[ -r /dev/i2c-1 ]] ||  echo "reboot required" && systemctl start argononed
}