#!/bin/sh

#
# Automatic install Zabbix agent
# © Kayama / 2016

wget http://repo.zabbix.com/zabbix/3.0/ubuntu/pool/main/z/zabbix/zabbix_3.0.0.orig.tar.gz

groupadd zabbix
useradd -g zabbix zabbix

tar -zxvf zabbix_3.0.0.orig.tar.gz

cd zabbix-3.0.0/

./configure --enable-agent

make install

sed -i 's/Server=127.0.0.1/Server=1.2.3.4/g' /usr/local/etc/zabbix_agentd.conf
sed -i 's/ServerActive=127.0.0.1/ServerActive=1.2.3.4/g' /usr/local/etc/zabbix_agentd.conf

cp ~/zabbix-3.0.0/misc/init.d/debian/zabbix-agent /etc/init.d/
update-rc.d zabbix-agent defaults

service zabbix-agent start

