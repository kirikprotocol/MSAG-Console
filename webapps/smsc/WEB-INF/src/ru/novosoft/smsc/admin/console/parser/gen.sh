#!/bin/sh

CLASSPATH=$CLASSPATH:$SMSC_SRCDIR/webapp/smsc/WEB-INF/lib/antlr.jar
CLASSOUTPATH=$SMSC_SRCDIR/webapp/smsc/WEB-INF/classes

echo Starting code generation ...
java -classpath $CLASSPATH antlr.Tool lexer.g
java -classpath $CLASSPATH antlr.Tool parser.g
echo Code generation done.

javac -classpath $CLASSPATH -d $CLASSOUTPATH *.java
