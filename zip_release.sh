#!/bin/bash
# Usage: 
#       $0 <checkout>


usage(){
    echo "Create releaes zips file from  source directories MCU, HCU and SCB at /IMAX_USER/Imaxeon-dev/ "
    echo "to the specified output directory excluding .hg"
    echo " the output filename is base on the current tag"
    echo "Usage: $0 <output_dir>"
    exit 1
}

if [ $# -lt 1 ]; then
    usage
fi

if [ ! -d "$1" ]; then
        echo "Invalid destionation directory"
        exit 2
fi
DST_DIR=$1

zip_hg_root_dir(){
    if [ ! -d "$2" ]; then
        echo "Invalid destination directory"
        exit 2
    fi

    if [ ! -d "$1" ]; then
        echo "Invalid source directory"
        exit 2
    fi
    
    if [ ! -d "$1/.hg" ]; then
        echo "Missing .hg in the source directory"
        exit 3
    fi

    if [ ! -d "$2" ]; then
        echo "Invalid output directory"
        exit 4
    fi


    src=`realpath "$1"`
    base=`basename "$src"`
    echo " src: $src"
    echo "base: $base"
    pushd .
    cd $src

    # cleanup .orig
    #echo "cleanup .orig"
    #find .  -type f | grep \.orig | xargs rm $1
    #echo "cleanup .orig done"

    # grep the last tag
    tag=`hg tags  | grep "^tip        " -A1 | grep -v "^tip   " | rev | cut -d " "  -f 2- | rev`
    tag=`echo $tag | sed -e 's/ /_/g'`
    tag=`echo $tag |cut -d "/" --output-delimiter=_ -f1-`
    echo "Tag: '$tag'"
    echo $src >> $2/zip_tag.txt
    hg tags  | grep "^tip        " -A1 | grep -v "^tip   "  >> $2/zip_tag.txt
    hg sum  >> $2/zip_tag.txt



    out=$2/${base}_${tag}.zip
    echo "Output zip: $out"

    # remove the existing zip
    [ -f $out ] && rm $out

    # zip but excludig .hg resoruce files
    zip -r $out * -x .hg/\* target/bin/resources/Lib/\* target/bin/resources/include\* target/bin/\*
    sha1sum $out >> $2/zip_tag.txt

    popd
}

zip_hg_root_dir /IMAX_USER/Imaxeon-dev/SY2-HCU $DST_DIR
zip_hg_root_dir /IMAX_USER/Imaxeon-dev/SY2-MCU $DST_DIR
zip_hg_root_dir /IMAX_USER/Imaxeon-dev/SY2-SCB $DST_DIR


