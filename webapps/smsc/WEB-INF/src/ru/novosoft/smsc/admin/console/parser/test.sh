#!/bin/sh

CLASSPATH=$CLASSPATH:$SMSC_SRCDIR/webapp/smsc/WEB-INF/lib/antlr.jar
CLASSOUTPATH=$SMSC_SRCDIR/webapp/smsc/WEB-INF/classes

java -classpath $CLASSPATH:$CLASSOUTPATH  ru.novosoft.smsc.admin.console.parser.Main 
