#!/usr/bin/env bash
# Author: Lesovsky A.V.
# Logical drives auto-discovery via MegaCLI. VERY VERY EXPERIMENTAL (TESTED WITH 8.02.21 Oct 21, 2011)

adp_list=$(sudo megacli adpallinfo aALL nolog |grep "^Adapter #" |cut -d# -f2)
ld_list=$(for a in $adp_list; do sudo megacli ldinfo lall a$a nolog |grep -w "^Virtual Drive:" |awk '{print $3}' |while read ld ; do echo $a:$ld; done ; done)

if [[ $1 = raw ]]; then
  for ld in ${ld_list}; do echo $ld; done ; exit 0
fi

echo -n '{"data":['
for ld in $ld_list; do echo -n "{\"{#LD}\": \"$ld\"},"; done |sed -e 's:\},$:\}:'
echo -n ']}'
