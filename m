#!/bin/bash
dira="/home/meesokim/spc1000/spcbios"
echo $0
if [ "$0" == "./m" ]; then
    spc1000="/home/meesokim/mame/mame64 spc1000 -uimodekey HOME -rompath /home/meesokim/mame/roms "
    param="-cass "
else    
    spc1000="/home/meesokim/fips-deploy/chips-test/win64-vs2017-debug/spc1000-ui"
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