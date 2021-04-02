#!/bin/bash
# MIT License

# Copyright (c) 2020 DarkElvenAngel

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# VERSION 0.2

echo -e "\e[H\e[J" 
echo -e "\e[37;1m    ___                                                __\e[0m"
echo -e "\e[37;1m   /   |  _________ _____  ____  ____  ____  ___  ____/ /\e[0m"
echo -e "\e[37;1m  / /| | / ___/ __ \`/ __ \/ __ \/ __ \/ __ \/ _ \/ __  / \e[0m"
echo -e "\e[37;1m / ___ |/ /  / /_/ / /_/ / / / / /_/ / / / /  __/ /_/ /  \e[0m"
echo -e "\e[37;1m/_/  |_/_/   \__, /\____/_/ /_/\____/_/ /_/\___/\__,_/   \e[0m"
echo -e "\e[37;1m            /____/                                       \e[0m"
echo -e "\e[37;1m                                                PACKAGER \e[0m"
echo "_________________________________________________________"
[[ -n $TARGET_DISTRO ]] && { ./configure || exit 1; }
[[ -f makefile.conf ]] || { echo -e "\e[31mERROR\e[0m:  run \e[1mTARGET_DISTRO=<NAME> ./package.sh\e[0m first"; exit 1; }
source makefile.conf
[[ -a OS/${DISTRO}/pkg_list ]] || { echo "ERROR:  \"${DISTRO}\" doesn't have a package list file, cannot generate package"; exit 1;}
OUT_FILENAME="build/${DISTRO}.pkg.sh"




echo -ne "\e[37;1mINFO:\e[0m  Preparing build environment ... "
make clean &> /dev/null && echo -e "\e[32mOK\e[0m" || { echo -e "\e[31mERR\e[0m\n\tSomething has gone wrong with \"make clean\" "; exit 1;}
rm ${OUT_FILENAME} &> /dev/null
echo -ne "\e[37;1mINFO:\e[0m  Building Source Files ... "
make &> /dev/null && echo -e "\e[32mOK\e[0m" || { echo -e "\e[31mERR\e[0m\n\tsomething has gone wrong with \"make\""; exit 1;}
echo -ne "\e[37;1mINFO:\e[0m  Checking files ... "
while read line; do
# reading each line
[[ -f $line ]] || { echo -e "\e[31mERR\nERROR\e[0m:  \e[1m${line}\e[0m File Not Found!"; exit 1; }
done < OS/${DISTRO}/pkg_list
echo -e "\e[32mOK\e[0m"

echo -ne "\e[37;1mINFO:\e[0m  Building Installer ... "
cat > ${OUT_FILENAME} <<SCRIPT_TOP
#!/bin/sh
echo "INFO:  ArgonOne Daemon self extracting installer"
DATA_START=\$((\`grep -an "^DATA_CONTENT$" \$0 | cut -d: -f1\` + 1))

F_EXTRACT()
{
	echo -n "INFO:  Extracting files ... "
	tail -n+\${DATA_START} \$0 | tar zxf - 2>/dev/null && echo "OK" || { echo "ERR"; exit 1; }
}
echo -n "INFO:  Checking installer ... "
#INSTALLER

type F_INSTALL &>/dev/null && echo "OK" || { echo -e "ERR\nERROR:  INSTALLER NOT FOUND"; exit 1;}
echo "INFO:  Starting installer"
F_INSTALL
exit 0
DATA_CONTENT
SCRIPT_TOP
[[ -f OS/${DISTRO}/pkg_install.sh ]] || { echo -e "\e[31mERR\nERROR\e[0m:  \e[1mOS/${DISTRO}/pkg_install.sh\e[0m NOT FOUND"; exit 1;}
sed -i -e "/#INSTALLER/r OS/${DISTRO}/pkg_install.sh" -e '/#INSTALLER/d' ${OUT_FILENAME}
echo -e "\e[32mOK\e[0m"
# tar -cvf allfiles.tar -T OS/${DISTRO}/pkg_list
echo -ne "\e[37;1mINFO:\e[0m  Packing files ... "
tar -T OS/${DISTRO}/pkg_list -czf - >> ${OUT_FILENAME} && echo -e "\e[32mOK\e[0m"

chmod +x ${OUT_FILENAME}

echo -ne "\e[37;1mINFO:\e[0m  Verify package ... "

while read line; do
# reading each line
tail -n+$((`grep -an "^DATA_CONTENT$" ${OUT_FILENAME} | cut -d: -f1` + 1)) ${OUT_FILENAME} | tar tzv $line &> /dev/null || { echo -e "\e[31mERR\nERROR\e[0m:  \e[1m${line}\e[0m File Not Found!"; exit 1; }
done < OS/${DISTRO}/pkg_list
echo -e "\e[32mOK\e[0m"

echo -e "\e[37;1mINFO\e[0m:  Package \e[1m${OUT_FILENAME}\e[0m is complete "

exit 0