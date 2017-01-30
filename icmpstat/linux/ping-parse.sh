#!/usr/bin/env bash
# Description:	Script for parse ping statistic between servers
# Kayama © 2017

NUMBER=0
FROMFILE=$1
SERVER=$2
METRIC=$3

[[ $# -lt 3 ]] && { echo "FATAL: some parameters not specified"; exit 1; }
[[ -f "$FROMFILE" ]] || { echo "FATAL: datafile not found"; exit 1; }

case "$3" in
"loss")
    NUMBER=2
;;
"avrg")
    NUMBER=3
;;
*) echo ZBX_NOTSUPPORTED; exit 1 ;;
esac

grep -w $SERVER $FROMFILE | tr -s ' ' | awk '{print $'$NUMBER'}'
