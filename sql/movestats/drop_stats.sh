if [ "$1" = "" ] ; then 
  echo Usage: drop_stats.sh period connect_string
  echo Examp: drop_stats.sh 20041019 \"smsc/smsc@ORCL\"
  exit 1
fi
if [ $1 -ge 999999 ] ; then 
  echo Error: period too big. Should be 6 symbols
  exit 2
fi
if [ "$2" = "" ] ; then 
  echo Usage: drop_stats.sh period connect_string
  echo Examp: drop_stats.sh 20041019 \"smsc/smsc@ORCL\"
  exit 1
fi
sed -e s/--PERIOD--/$1/g stats_drop_template.sql > stats_drop.sql
sqlplus $2 @stats_drop.sql

rm -f stats_drop.sql
