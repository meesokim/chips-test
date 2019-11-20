#!/bin/bash
me=`whoami`
dira="/home/$me/spc1000/spcbios"
echo $0
if [ "$0" == "./m" ]; then
    spc1000="/home/$me/mame/mame64 spc1000 -uimodekey HOME -rompath /home/$me/mame/roms "
    param="-cass "
else    
    spc1000="/home/$me/fips-deploy/chips-test/win64-vstudio-release/spc1000-ui"
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