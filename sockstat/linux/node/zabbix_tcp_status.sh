#!/bin/bash
HOST=`/bin/hostname`
SERVER='zabbix.server.com'
/bin/ss -ant | awk "{if (NR>1) {state[\$1]++}} END {host = \"${HOST}\"; \
for (i in state) {s=i; \
sub (/ESTAB/, \"establ\", s); sub (/LISTEN/, \"listen\", s); sub (/SYN-SENT/, \"synsent\", s);  \
sub (/SYN-RECV/, \"synrecv\", s); sub (/FIN-WAIT-1/, \"finw1\", s); sub (/FIN-WAIT-2/, \"finw2\", s); \
sub (/CLOSE-WAIT/, \"closew\", s); sub (/TIME-WAIT/, \"timew\", s); print host, \"tcp.\"s, state[i]}}" \
| /usr/local/bin/zabbix_sender --zabbix-server ${SERVER} -s ${HOST} --port '10051' -i - >/dev/null 2>&1
echo "1"
exit 0
