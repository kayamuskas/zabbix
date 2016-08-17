#!/usr/bin/env bash

SECONDS=$2
TOFILE=$1
IOSTAT="/usr/bin/iostat"

[[ $# -lt 2 ]] && { echo "FATAL: some parameters not specified"; exit 1; }

$IOSTAT -x 1 $SECONDS | grep -v 'extended' | awk 'BEGIN {check=0;} {if(check==1 && $1!=""){print $0}if($1=="device"){check=1}}' > $TOFILE.new 
mv $TOFILE.new $TOFILE
echo 0
