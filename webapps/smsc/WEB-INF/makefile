ifndef SMSC_BUILDDIR
SMSC_BUILDDIR:=$(HOME)/build/smsc
$(warning set SMSC_BUILDDIR = $(SMSC_BUILDDIR))
export SMSC_BUILDDIR
endif

SERVLET_JAR := /export/home/smsc/tomcat/common/lib/servlet.jar
CLASSPATH := lib/classes12.jar:lib/jakarta-log4j-1.0.4-class.jar:lib/nsjsp.jar:$(SERVLET_JAR):

all:
	javac -sourcepath src -classpath $(CLASSPATH) -d classes `find src -name *.java`
	jar -cf smsc.jar -C classes .
	mv -f smsc.jar $(SMSC_BUILDDIR)/jars/smsc.jar