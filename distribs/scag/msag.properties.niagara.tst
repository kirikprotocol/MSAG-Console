installation.type=single
installation.mirrorpath=

catalina.control.port=23800
catalina.connector.port=23801
logger.default.category=DEBUG
logs.dir=logs

WHOISD.url-pattern=/WHOISD/*

scag.service.name=scag
scag.service.args=MSAG v1
scag.service.autostart=false

scag2.service.name=msag
scag2.service.args=MSAG v2
scag2.service.autostart=true

listen.host=0.0.0.0
connect.host=127.0.0.1

daemon.admin.port=23810
daemon.dir               =daemon
daemon.hsadmin.link.name =msag_admin
#daemon.bin.name         =hs_smsc_ssdaemon
#daemon.link.name        =hs_msag_ssdaemon
daemon.bin.name          =smsc_ssdaemon
daemon.link.name         =msag_ssdaemon

gateway.admin.port=23811
gateway.smpp.port=23830
gateway.http.port=23840

bill.config_dir=conf
bill.server_host=127.0.0.1
bill.server_port=23989
bill.time_out=1
bill.reconnect_time_out=60

status.show_interval=86400000
language.default=ru

scag.session.location=/opt/home/msag_test/msag_distr/storage/sessions
scag2.session.location=/opt/home/msag_test/msag_distr/storage/sessions
session.expireInterval=60000

logger.properties_file=/opt/home/msag_test/msag_distr/services/scag/logger.properties

rules.path      =/opt/home/msag_test/msag_distr/services/scag/conf/rules
stat.dir        =/opt/home/msag_test/msag_distr/storage/statistics
stat.traffic_dir=/opt/home/msag_test/msag_distr/storage/statistics/traffic

stat.perf_host=127.0.0.1
stat.perf_port=23981
stat.perf_applet.port=23982
stat.perf_svc_port=23983
stat.perf_svc_applet.port=23984
stat.perf_sc_port=23985
stat.perf_sc_applet.port=23986

stat.sacc_port=23700
stat.sacc_host=127.0.0.1
stat.sacc_connect_timeout=60
stat.sacc_queue_length=1000

pers_client.io_timeout=100
pers_client.reconnect_timeout=100
pers_client.ping_timeout=100
pers_client.max_waiting_requests_count=100
pers_client.connections=5
pers_client.async=true

pers.port=23880
pers.listen_host=0.0.0.0
pers.connect_host=127.0.0.1
pers.connections=100
pers.io_timeout=300
pers.ping_timeout=300
pers.storage_dir=/opt/home/msag_test/msag_distr/storage/pers
pers.hierarchical_mode=false
pers.central_host=127.0.0.1
pers.central_port=23890
pers.transact_timeout=300
pers.region_id=1
pers.region_password=psw1
pers.service.args=MSAG PERS
pers.service.autostart=false

cpers.listen_host=0.0.0.0
cpers.port=23890
cpers.connections=100
cpers.storage_dir=/opt/home/msag_test/msag_distr/storage/cpers
cpers.transact_timeout=200
cpers.regions_config_name=/opt/home/msag_test/msag_distr/services/cpers/conf/region.xml
cpers.service.args=MSAG PVCP
cpers.service.autostart=false

pvss.port=23880
pvss.listen_host=0.0.0.0
pvss.connect_host=127.0.0.1
pvss.timeout=300
pvss.io_pool_size=10
pvss.connections=100
pvss.perf_counter_on=false
pvss.perf_counter_period=10
pvss.storage_dir=/opt/home/msag_test/msag_distr/storage/pvss
pvss.location1_path=/opt/home/msag_test/msag_distr/storage/pvss/1
pvss.storages=100
pvss.nodes=1
pvss.node=0
pvss.storage_queue_size=20
pvss.abonent_storage_name=abonent
pvss.infrastruct_storage_dir=/opt/home/msag_test/msag_distr/storage/pvss/infrastruct
pvss.service.args=MSAG PVSS
pvss.service.autostart=true

rule_editor.ping_port=23987
rule_editor.ping_timeout=10000

LC_ALL=ru_RU.WINDOWS-1251
NLS_LANG=RUSSIAN_CIS.CL8MSWIN1251
JAVA_HOME=/usr/jdk/j2sdk1.4.2_08
JAVA_OPTS=-Dmsag_site -Xms64m -Xmx256m -Dfile.encoding=windows-1251 -Duser.timezone=Asia/Novosibirsk
