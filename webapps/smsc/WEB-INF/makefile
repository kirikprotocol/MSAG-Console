ifndef SMSC_BUILDDIR
SMSC_BUILDDIR:=$(HOME)/build/smsc
$(warning set SMSC_BUILDDIR = $(SMSC_BUILDDIR))
export SMSC_BUILDDIR
endif

TOMCAT_DIR := /export/home/smsc/tomcat
SERVLET_JAR := $(TOMCAT_DIR)/common/lib/servlet.jar
CATALINA_JAR := $(TOMCAT_DIR)/server/lib/catalina.jar

CLASSPATH := lib/classes12.jar:lib/jakarta-log4j-1.0.4-class.jar:lib/nsjsp.jar:$(SERVLET_JAR):$(CATALINA_JAR)

all: compile
rebuild: clean compile

compile:
	@echo compiling SMSC
	@javac -sourcepath src -classpath $(CLASSPATH) -d classes `find src -name *.java`
	@echo making jar
	@mkdir -p $(SMSC_BUILDDIR)/jars
	@jar -cf $(SMSC_BUILDDIR)/jars/smsc_tomcat_server.jar -C classes ru/novosoft/smsc/util/auth/SmscRealm.class
	@rm classes/ru/novosoft/smsc/util/auth/SmscRealm.class
	@jar -cf $(SMSC_BUILDDIR)/jars/smsc_tomcat_common.jar -C classes ru/novosoft/smsc/util/auth/Authenticator.class -C classes ru/novosoft/smsc/util/auth/AuthenticatorProxy.class
	@rm classes/ru/novosoft/smsc/util/auth/Authenticator.class classes/ru/novosoft/smsc/util/auth/AuthenticatorProxy.class
	@jar -cf $(SMSC_BUILDDIR)/jars/smsc.jar -C classes .

clean:
	@echo clean
	@rm -rf classes/ru
	@rm -rf $(SMSC_BUILDDIR)/jars/smsc_tomcat_server.jar
	@rm -rf $(SMSC_BUILDDIR)/jars/smsc_tomcat_common.jar
	@rm -rf $(SMSC_BUILDDIR)/jars/smsc.jar
