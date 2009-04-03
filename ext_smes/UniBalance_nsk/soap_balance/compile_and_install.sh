#mvn clean
mvn -Dmaven.test.skip package
rm -rf tomcat/webapps/service_webapp
mv service_webapp/target/service_webapp tomcat/webapps/service_webapp