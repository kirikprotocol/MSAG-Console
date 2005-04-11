#!/bin/sh

CLASSOUTPATH=$SMSC_SRCDIR/../webapps/smsc/WEB-INF/classes

CLASSPATH=$CLASSPATH:$SMSC_SRCDIR/../webapps/libs/antlr.jar:$CLASSOUTPATH

echo Starting code generation ...
java -classpath $CLASSPATH antlr.Tool lexer.g
java -classpath $CLASSPATH antlr.Tool parser.g
echo Code generation done.

#javac -classpath $CLASSPATH -d $CLASSOUTPATH *.java
