if [ "$1" = "" ] ; then 
  echo Usage: drop_stats.sh period connect_string
  echo Examp: drop_stats.sh 20041019 \"smsc/smsc@ORCL\"
  exit 1
fi
if [ "$2" = "" ] ; then 
  echo Usage: drop_stats.sh period connect_string
  echo Examp: drop_stats.sh 20041019 \"smsc/smsc@ORCL\"
  exit 1
fi
sed -e s/--PERIOD--/$1/ stats_drop_template.sql > stats_drop.sql
sqlplus $2 @stats_drop.sql
rm -f stats_drop.sql
