#!/bin/bash

cls=conf
for i in lib/*.jar ; do cls=$cls:$i ; done
java -cp $cls mobi.eyeline.mcahdb.MCAHDB $@