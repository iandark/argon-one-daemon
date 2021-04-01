#!/bin/bash
# Auto installer for argononed this will grab the latest version
function version { echo "$@" | awk -F. '{ printf("%03d%03d%03d\n", $1,$2,$3); }'; }

echo -e "\e[H\e[J" 
echo -e "\e[37;1m    ___                                                __\e[0m"
echo -e "\e[37;1m   /   |  _________ _____  ____  ____  ____  ___  ____/ /\e[0m"
echo -e "\e[37;1m  / /| | / ___/ __ \`/ __ \/ __ \/ __ \/ __ \/ _ \/ __  / \e[0m"
echo -e "\e[37;1m / ___ |/ /  / /_/ / /_/ / / / / /_/ / / / /  __/ /_/ /  \e[0m"
echo -e "\e[37;1m/_/  |_/_/   \__, /\____/_/ /_/\____/_/ /_/\___/\__,_/   \e[0m"
echo -e "\e[37;1m            /____/                                       \e[0m"
echo -ne "\n\e[37;1m Auto Installer\n\e[0m"
echo "_________________________________________________________"
echo -e 
if [ "$EUID" -ne 0 ]
then
    echo -e "\e[37;1mYou are running without root permissions\e[0m"
    echo "If any task requires root access sudo will be used."
    read -e -p "is this OK [y/N]?" choice
    [[ "$choice" == [Yy]* ]] || exit 1
fi
command -v git &> /dev/null && GIT=1 || GIT=0
if [ $GIT -eq 0 ]
then 
    command -v apt &> /dev/null && APT=1 || APT=0
    [[ APT -eq 0 ]] && { echo "please install git and run again"; exit 1; }
    echo -e "\e[37;1mgit\e[0m isn't installed on your system."
    read -e -p "install package [y/N]?" choice
    if [[ "$choice" == [Yy]* ]]
    then 
        echo -n "Installing git ... "
        if [ "$EUID" -ne 0 ]
            then sudo apt install git -y &>/dev/null && echo "OK!" || { echo "FAILED!"; exit 1; }
            else apt install git -y &>/dev/null && echo "OK!" || { echo "FAILED!"; exit 1; }
        fi
    else
        echo "ERROR:  Install cannot complete"
        exit 1
    fi
fi
echo -e "\e[1m>>> Checking Available Versions \e[0m"
CURRENT_PATH=${PWD##*/}
FOUND=0
if [ "$CURRENT_PATH" == "argononed" ]
then
    if [ -f "version" ]
    then
        CURRENT_VERSION=$(head -n1 version)
    else
        CURRENT_VERSION="0.1.5"
    fi
    FOUND=1
else
    CURRENT_VERSION="0.0.0"
    if [ -d "argononed" ]
    then
        if [ -f "argononed/version" ]
        then
            CURRENT_VERSION=$(head -n1 argononed/version)
            FOUND=2
        else
            CURRENT_VERSION="0.1.5"
            FOUND=2
        fi
    fi
fi
UPDATE=0
MASTERBRANCH_VERSION=$(curl -s https://gitlab.com/DarkElvenAngel/argononed/-/raw/master/version)
LATESTBRANCH_VERSION=$(curl -s https://gitlab.com/DarkElvenAngel/argononed/-/raw/0.3.x/version)
if [ "$CURRENT_VERSION" == "0.0.0" ]
then
    echo "Installed version     [ NONE ]" 
else
    echo "Installed version     [ $CURRENT_VERSION ]" 
fi
echo "Latest STABLE version [ $MASTERBRANCH_VERSION ]"
echo "Latest Branch version [ $LATESTBRANCH_VERSION ]"
if [ "$(version "$CURRENT_VERSION")" -lt "$(version "$MASTERBRANCH_VERSION")" ]
then
    echo -e "\e[37;1mNew version availiable!\e[0m"
    UPDATE=1
else
    echo -e "\e[37;1mNo update required\e[0m."
fi
if [ "$(version "$CURRENT_VERSION")" -lt "$(version "$LATESTBRANCH_VERSION")" ]
then
    echo -e "\e[37;1mnewer testing version availiable!\e[0m"
    let "UPDATE|=2"
else
    echo -e "\e[37;1mNo newer versions available\e[0m"
fi
if [[ $FOUND -ne 0 && $UPDATE -eq 0 ]]
then
    exit 0
fi
echo -e "\e[1m>>> Getting Files \e[0m"
case $FOUND in
    0) 
        git clone  https://gitlab.com/DarkElvenAngel/argononed.git || exit $?
        cd argononed
        ;;
    2)
        cd argononed
        ;&
    1)
        git pull &> /dev/null
        git checkout master &> /dev/null
        ;;
    *)
        echo "Something has gone wrong!"
        exit 2;
esac
if [ $(($UPDATE & 1)) -ne 0 ]
then 
    CHOICE="1"
    echo "1 ] Install Stable version"
fi
if [ $(($UPDATE & 2)) -ne 0 ]
then
    echo "2 ] Install Testing version"
    CHOICE="${CHOICE}2"
fi
echo "x ] Do nothing and quit"
echo $CHOICE
while true; do
    read -e -p "> " choice
    case $choice in 
        "1" ) 
            if [[ $CHOICE == *"$choice"* ]]
            then
                break
            else
                echo "Invalid choice" 
            fi
            ;;
        "2" ) 
            if [[ $CHOICE == *"$choice"* ]]
            then
                git checkout 0.3.x &> /dev/null
                break
            else
                echo "Invalid choice" 
            fi
            ;;
        "x" )
            exit 0
            ;;
        * ) echo "Invalid choice" ;; 
    esac
done
echo -e "\e[1m>>> Reset build to mrproper...\e[0m"
make mrproper || { echo -e "\e[31;1m>>> make ERROR...\e[0m"; exit 1; }
echo -e "\e[1m>>> Running prebuild configuration...\e[0m"
./configure $@
if [ $? -ne 0 ]
then
    echo -e "\e[31;1m>>> Configuration ERROR...\e[0m"
    exit 1;
fi
echo -e "\e[1m>>> Building...\e[0m"
make all $@
if [ $? -ne 0 ]
then
    echo -e "\e[31;1m>>> Build ERROR...\e[0m"
    exit 1;
fi
echo -e "\e[1m>>> Installing...\e[0m"
if [ "$EUID" -ne 0 ]
    then sudo make install || exit $?
    else make install || exit $?
fi
echo -e "\e[1m>>>  Complete\e[0m"

exit 0;

