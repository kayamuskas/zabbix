wget http://www.zabbix.com/downloads/3.0.4/zabbix_agents_3.0.4.solaris10.amd64.tar.gz

mkdir /opt/zabbix
tar -zxvf zabbix_agents_3.0.4.solaris10.amd64.tar.gz -C /opt/zabbix/

sed -i 's/Server=127.0.0.1/Server=example.com/g' /opt/zabbix/conf/zabbix_agentd.conf 
sed -i 's/ServerActive=127.0.0.1/ServerActive=example.com/g' /opt/zabbix/conf/zabbix_agentd.conf 

mkdir /opt/zabbix/etc
ln -s /opt/zabbix/conf/zabbix_agentd.conf /opt/zabbix/etc/zabbix_agentd.conf

echo 'zabbix:x:42:2:Zabbix Agent:/:/bin/sh' >> /etc/passwd

svccfg import zabbix_agent.xml
