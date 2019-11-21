#!/bin/bash
me=`whoami`
a=`cat .fips-settings.yml`
a=${a:9}
a=${a/\}/}
dira="/home/$me/spc1000/spcbios"
if [ "$0" == "./m" ]; then
    spc1000="/home/$me/mame/mame64 spc1000 -uimodekey HOME -rompath /home/$me/mame/roms "
    param="-cass "
else
    if [[ $a == "*wasm*" ]]; then
        spc1000="python3 -m http.server 80 --directory /home/$me/fips-deploy/chips-test/$a/"
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
        $spc1000 file=$dira/$1;
    else
        mapfile -t files < <(ls $dira/*.tap $dira/*.cas)
        tap="${files[$1]}"
        echo $spc1000 $param"$tap"
        exec $spc1000 $param"$tap";
    fi
fi 