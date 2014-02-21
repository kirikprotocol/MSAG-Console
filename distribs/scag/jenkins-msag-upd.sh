rm -rf $WORKSPACE/.distr || true
mkdir $WORKSPACE/.distr || true

echo "21.02.2014 https://ms.eyeline.mobi:9443/show_bug.cgi?id=955" > $WORKSPACE/.distr/readme.txt

mkdir $WORKSPACE/.distr/lib || true
mkdir $WORKSPACE/.distr/services || true
mkdir $WORKSPACE/.distr/services/msag || true
mkdir $WORKSPACE/.distr/services/msag/bin || true
mkdir $WORKSPACE/.distr/webapps || true
mkdir $WORKSPACE/.distr/webapps/msag || true
mkdir $WORKSPACE/.distr/webapps/msag/WEB-INF || true
mkdir $WORKSPACE/.distr/webapps/msag/WEB-INF/lib || true
mkdir $WORKSPACE/.distr/webapps/msag/WEB-INF/classes || true
mkdir $WORKSPACE/.distr/webapps/msag/WEB-INF/classes/locales || true
mkdir $WORKSPACE/.distr/webapps/msag/routing || true
mkdir $WORKSPACE/.distr/webapps/msag/routing/subjects || true

cp $WORKSPACE/.build/msag/bin/liblogger.so $WORKSPACE/.distr/lib
cp $WORKSPACE/.build/msag/bin/scag/msag $WORKSPACE/.distr/services/msag/bin
if [ "-$XERCES_HOME" != "-" ] ; then
cp $XERCES_HOME/lib/*.so $WORKSPACE/.distr/lib
fi

cp $WORKSPACE/.build/msag-web/exploded/msag/routing/subjects/edit.jsp $WORKSPACE/.distr/webapps/msag/routing/subjects
cp $WORKSPACE/.build/msag-web/classes/msag/locales/* $WORKSPACE/.distr/webapps/msag/WEB-INF/classes/locales
cp $WORKSPACE/.build/msag-web/exploded/msag/WEB-INF/lib/scag.jar $WORKSPACE/.distr/webapps/msag/WEB-INF/lib
