ifndef SMSC_BUILDDIR
SMSC_BUILDDIR:=$(HOME)/build/smsc
$(warning set SMSC_BUILDDIR = $(SMSC_BUILDDIR))
export SMSC_BUILDDIR
endif

TOMCAT_DIR := /export/home/smsc/tomcat
SERVLET_JAR := $(TOMCAT_DIR)/common/lib/servlet.jar
CATALINA_JAR := $(TOMCAT_DIR)/server/lib/catalina.jar

CLASSPATH := lib/classes12.jar:lib/jakarta-log4j-1.0.4-class.jar:lib/nsjsp.jar:$(SERVLET_JAR):$(CATALINA_JAR):lib/antlr.jar

all: compile
rebuild: clean compile

compile:
	@echo compiling SMSC
	@javac -sourcepath src -classpath $(CLASSPATH) -d classes `find src -name *.java`
	@native2ascii -encoding windows-1251 <src/locales/messages_en.properties > classes/locales/messages_en.properties
	@native2ascii -encoding windows-1251 <src/locales/messages_ru.properties > classes/locales/messages_ru.properties
	@native2ascii -encoding windows-1251 <src/ru/novosoft/smsc/admin/console/commands/roles_en_US.properties > classes/ru/novosoft/smsc/admin/console/commands/roles_en_US.properties
	@native2ascii -encoding windows-1251 <src/ru/novosoft/smsc/admin/console/commands/roles_ru_RU.properties > classes/ru/novosoft/smsc/admin/console/commands/roles_ru_RU.properties
	@native2ascii -encoding windows-1251 <src/ru/novosoft/smsc/admin/console/commands/roles_en_US.properties > classes/ru/novosoft/smsc/admin/console/commands/roles_en.properties
	@native2ascii -encoding windows-1251 <src/ru/novosoft/smsc/admin/console/commands/roles_ru_RU.properties > classes/ru/novosoft/smsc/admin/console/commands/roles_ru.properties
	@echo making jar
	@mkdir -p $(SMSC_BUILDDIR)/jars
	@jar -cf $(SMSC_BUILDDIR)/jars/smsc_tomcat_server.jar -C classes ru/novosoft/smsc/util/auth/SmscRealm.class
	@rm classes/ru/novosoft/smsc/util/auth/SmscRealm.class
	@jar -cf $(SMSC_BUILDDIR)/jars/smsc_tomcat_common.jar -C classes ru/novosoft/smsc/util/auth/Authenticator.class -C classes ru/novosoft/smsc/util/auth/AuthenticatorProxy.class
	@rm classes/ru/novosoft/smsc/util/auth/Authenticator.class classes/ru/novosoft/smsc/util/auth/AuthenticatorProxy.class
	@native2ascii -encoding windows-1251 <src/ru/novosoft/smsc/perfmon/applet/text_en_US.properties > classes/ru/novosoft/smsc/perfmon/applet/text_en_US.properties
	@native2ascii -encoding windows-1251 <src/ru/novosoft/smsc/perfmon/applet/text_ru_RU.properties > classes/ru/novosoft/smsc/perfmon/applet/text_ru_RU.properties
	@jar -cf $(SMSC_BUILDDIR)/jars/perfmon.jar -C classes ru/novosoft/smsc/perfmon/applet/ -C classes ru/novosoft/smsc/perfmon/PerfSnap.class
	@rm -rf classes/ru/novosoft/smsc/perfmon/applet
	@jar -cf $(SMSC_BUILDDIR)/jars/smsc.jar -C classes .

clean:
	@echo clean
	@rm -rf classes/ru
	@rm -rf $(SMSC_BUILDDIR)/jars/smsc_tomcat_server.jar
	@rm -rf $(SMSC_BUILDDIR)/jars/smsc_tomcat_common.jar
	@rm -rf $(SMSC_BUILDDIR)/jars/perfmon.jar
	@rm -rf $(SMSC_BUILDDIR)/jars/smsc.jar
