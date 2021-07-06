#! /bin/bash
[ $# -eq 0 ] && FILE=/boot/config.txt || FILE=$1
[ -w "$FILE" ] || { echo "ERROR Cannot Write to ${FILE} unable to continue"; exit 1; }
SYSMODEL=$( awk '{ print $0 }' /proc/device-tree/model | sed 's|Raspberry Pi||;s|Rev.*||;s|Model||;s|Zero|0|;s|Plus|+|;s|B| |;s|A| |;s| ||g' )

echo -n "Search config.txt for i2c_arm ... "
grep -i '^dtparam=i2c_arm=on' $FILE 1> /dev/null && { echo "FOUND"; exit 0; } || echo "NOT FOUND"
echo -n "Insert i2c_arm into ${FILE} ... "
if [[ `grep -i "^\[pi${SYSMODEL}\]" $FILE` ]]
then
    sed  -i "/^\[pi${SYSMODEL}\]/a dtparam=i2c_arm=on" $FILE && echo "DONE";
else
    echo "dtparam=i2c_arm=on" >> $FILE && echo "DONE";
fi
echo -n "Search /etc/modules for i2c-dev ... "
grep -i '^i2c-dev' /etc/modules 1> /dev/null && { echo "FOUND"; exit 0; } || echo "NOT FOUND"
echo -n "Insert i2c-dev into /etc/modules "
echo "i2c-dev" >> /etc/modules && echo "DONE";
exit 0