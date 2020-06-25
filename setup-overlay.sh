#! /bin/bash
[ $# -eq 0 ] && FILE=/boot/config.txt || FILE=$1
[ -w "$FILE" ] || { echo "ERROR Cannot Write to ${FILE} unable to continue"; exit 1; }
SYSMODEL=$( awk '{ print $0 }' /proc/device-tree/model | sed 's|Raspberry Pi||;s|Rev.*||;s|Model||;s|Zero|0|;s|Plus|+|;s|B| |;s|A| |;s| ||g' )

echo -n "Search config.txt for overlay ... "
grep -i '^dtoverlay=argonone' $FILE 1> /dev/null && { echo "FOUND"; exit 0; } || echo "NOT FOUND"
cp $FILE $FILE.backup
echo -n "Insert overlay into /boot/config.txt ... "
if [[ `grep -i "^\[pi${SYSMODEL}\]" $FILE` ]]
then
    sed  -i "/^\[pi${SYSMODEL}\]/a dtoverlay=argonone" $FILE && echo "DONE";
else
    echo "dtoverlay=argonone" >> $FILE && echo "DONE";
fi
exit 0
