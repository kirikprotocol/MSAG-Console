ifndef SMSC_BUILDDIR
SMSC_BUILDDIR:=$(HOME)/build/smsc
$(warning set SMSC_BUILDDIR = $(SMSC_BUILDDIR))
export SMSC_BUILDDIR
endif

SERVLET_JAR := /export/home/smsc/tomcat/common/lib/servlet.jar
CLASSPATH := lib/classes12.jar:lib/jakarta-log4j-1.0.4-class.jar:lib/nsjsp.jar:$(SERVLET_JAR):

all: compile
rebuild: clean compile

compile:
	javac -sourcepath src -classpath $(CLASSPATH) -d classes `find src -name *.java`
	jar -cf $(SMSC_BUILDDIR)/jars/smsc.jar -C classes .

clean:
	rm -rf classes/ru
	rm -rf $(SMSC_BUILDDIR)/jars/smsc.jar