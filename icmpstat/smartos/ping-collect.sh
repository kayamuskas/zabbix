#!/usr/bin/env bash
# Description:	Script for collect ping statistic between servers
# Kayama © 2017

TOFILE=$1

ARG1=$2
ARG2=$3
ARG3=$4

rm -f $TOFILE.new

[[ $# -lt 1 ]] && { echo "FATAL: some parameters not specified"; exit 1; }

SERVERS="$ARG1 $ARG2 $ARG3"

for SERVER in $SERVERS
    do

    PING=$(/opt/local/sbin/fping -c 3 $SERVER 2>&1 | tail -n 1 | awk -F/ '{print $5, $8}' | cut -d\  -f1,3 | sed 's/,//g; s/%//g;')
    echo "$SERVER $PING" >> $TOFILE.new

    done

mv $TOFILE.new $TOFILE

echo 1
