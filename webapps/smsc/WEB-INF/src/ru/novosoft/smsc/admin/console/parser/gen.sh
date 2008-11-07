#!/bin/sh

CLASSOUTPATH=/home/artem/Work/projects/smsc/webapps/smsc/WEB-INF/classes

CLASSPATH=$CLASSPATH:/home/artem/Work/projects/smsc/webapps/libs/antlr.jar:$CLASSOUTPATH

echo Starting code generation ...
java -classpath $CLASSPATH antlr.Tool lexer.g
java -classpath $CLASSPATH antlr.Tool parser.g
echo Code generation done.

#javac -classpath $CLASSPATH -d $CLASSOUTPATH *.java
