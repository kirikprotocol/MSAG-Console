cls=conf
for i in lib/*.jar ; do cls=$cls:$i ; done
java -Dfile.encoding=windows-1251 -classpath $cls com.sibinco.emailsender.McismeStatReport --prop_file conf/config.properties --send_mail $1 $2
`