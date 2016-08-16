#!/usr/bin/env bash

NUMBER=0
FROMFILE=$1
DISK=$2
METRIC=$3

[[ $# -lt 3 ]] && { echo "FATAL: some parameters not specified"; exit 1; }
[[ -f "$FROMFILE" ]] || { echo "FATAL: datafile not found"; exit 1; }

case "$3" in
"r/s")
        NUMBER=2
;;
"w/s")
        NUMBER=3
;;
"kr/s")
        NUMBER=4
;;
"kw/s")
        NUMBER=5
;;
"wait")
        NUMBER=6
;;
"actv")
        NUMBER=7
;;
"svc_t")
        NUMBER=8
;;
"queue")
        NUMBER=9
;;
"busy")
        NUMBER=10
;;
*) echo ZBX_NOTSUPPORTED; exit 1 ;;
esac

grep -w $DISK $FROMFILE | tail -n +2 | tr -s ' ' |awk -v N=$NUMBER 'BEGIN {sum=0.0;count=0;} {sum=sum+$N;count=count+1;} END {printf("%.2f\n", sum/count);}'
