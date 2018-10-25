#!/usr/bin/env bash
# Author:	Lesovsky A.V.
# Description:	Physical drive auto-discovery via arcconf. (TESTED ON V5.20 (B17414))

adp_list=$(sudo arcconf getversion |grep -w "^Controller" |cut -d# -f2)
pd_list=$(for a in $adp_list; 
            do
              sudo arcconf getconfig 1 pd |grep -B1 -w "Device is a Hard drive" |grep -wE "Device.*[0-9]+" |cut -d# -f2 |awk -v adp=$a '{print adp":"$1}' 
            done)

if [[ $1 = raw ]]; then
  for pd in ${pd_list}; do echo $pd; done ; exit 0
fi

echo -n '{"data":['
for pd in $pd_list; do echo -n "{\"{#PD}\": \"$pd\"},"; done |sed -e 's:\},$:\}:'
echo -n ']}'
