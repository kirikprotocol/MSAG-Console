#!/bin/bash

cd ..

cls=conf
for i in webapps/smsx/WEB-INF/lib/*.jar ; do cls=$cls:$i ; done

java -Dremove_group_script -cp $cls -Xmx128M -Dfile.encoding=windows-1251 ru.sibinco.smsx.engine.service.group.RemoveGroupScript $@