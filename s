#!/bin/bash
me=`whoami`
a=`cat .fips-settings.yml`
a=${a:9}
a=${a/\}/}
dira="/home/$me/spc1000/goodtapes"
if [ "$0" == "./m" ]; then
    spc1000="/home/$me/mame/mame64 spc1000 -uimodekey HOME -rompath /home/$me/mame/roms "
    param="-cass "
else
    if [[ $a == "*wasm*" ]]; then
        spc1000="python3 -m http.server 80 --directory /home/$me/fips-deploy/chips-test/$a/"
    elif [[ $a == *"droid"* ]]; then
        spc1000="../fips-sdks/android/platform-tools/adb install /home/$me/fips-deploy/chips-test/$a/spc1000-ui.apk"
    else
        spc1000="/home/$me/fips-deploy/chips-test/$a/spc1000-ui"
    fi
    param="file="
fi
if [ "$1" == "" ]; then
    $spc1000;
else
    re='^[0-9]+$'
    if ! [[ $1 =~ $re ]] ; then
        if test -f "$dira/$1"; then
            $spc1000 $param"$dira/$1";
        else
            echo file not found: $dira/$1
        fi
    else
        mapfile -t files < <(ls $dira/*.tap $dira/*.cas)
        tap="${files[$1]}"
        echo "$tap"
        exec $spc1000 $param"$tap";
    fi
fi 