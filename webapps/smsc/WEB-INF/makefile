SERVLET_JAR := /export/home/smsc/tomcat/common/lib/servlet.jar
CLASSPATH := lib/classes12.jar:lib/jakarta-log4j-1.0.4-class.jar:lib/nsjsp.jar:$(SERVLET_JAR):

all:
	javac -sourcepath src -classpath $(CLASSPATH) -d classes `find src -name *.java`
