export CATALINA_HOME=.
export CATALINA_BASE=.
./bin/catalina.sh start 1>logs/tomcat.out 2>logs/tomcat.err
./deploy.sh