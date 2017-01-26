#!/bin/bash
HOST=`/bin/hostname`
SERVER='zabbix.server.com'
netstat -an -f inet -P tcp | grep -v '\-\-\-\-\-' | grep -v 'TCP: IPv4' | grep -v 'State' | awk "{if (NR>1) {state[\$7]++}} END {host = \"${HOST}\"; \
for (i in state) {s=i; \
sub (/ESTABLISHED/, \"establ\", s); sub (/LISTEN/, \"listen\", s); sub (/SYN_SENT/, \"synsent\", s);  \
sub (/SYN_RCVD/, \"synrecv\", s); sub (/FIN_WAIT_1/, \"finw1\", s); sub (/FIN_WAIT_2/, \"finw2\", s); \
sub (/CLOSING/, \"closing\", s); sub (/CLOSE_WAIT/, \"closew\", s); sub (/TIME_WAIT/, \"timew\", s); \
sub (/LAST_ACK/, \"lastack\", s); print host, \"tcp.\"s, state[i]}}" \
| /opt/zabbix/bin/zabbix_sender --zabbix-server ${SERVER} -s ${HOST} --port '10051' -i - >/dev/null 2>&1
echo "1"
exit 0
