SERVLET_JAR := /export/home/smsc/tomcat/common/lib/servlet.jar
CLASSPATH := ../../lib/classes12.jar:../../lib/jakarta-log4j-1.0.4-class.jar:$(SERVLET_JAR):$(SMSC_BUILDDIR)/jars/smsc.jar:$(SMSC_SRCDIR)/webapp/smsc/WEB-INF/lib/nsjsp.jar


all: compile


compile:
	@echo Compile
	@mkdir -p $(SMSC_BUILDDIR)/classes
	@mkdir -p $(SMSC_BUILDDIR)/jars
	@javac -sourcepath . -classpath $(CLASSPATH) -d $(SMSC_BUILDDIR)/classes `find . -follow -name *.java`
	@echo create jar
	@jar -cf $(SMSC_BUILDDIR)/jars/wsme.jar -C $(SMSC_BUILDDIR)/classes .

clean:
	@echo Clean
	@rm -rf $(SMSC_BUILDDIR)/classes
	@rm -f $(SMSC_BUILDDIR)/jars/wsme.jar

rebuild: clean compile
