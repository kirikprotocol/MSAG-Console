rm -rf $WORKSPACE/.distr || true
mkdir $WORKSPACE/.distr || true
mkdir $WORKSPACE/.distr/lib || true
mkdir $WORKSPACE/.distr/services || true
mkdir $WORKSPACE/.distr/services/msag || true
mkdir $WORKSPACE/.distr/services/msag/bin || true
mkdir $WORKSPACE/.distr/services/msag/conf || true
mkdir $WORKSPACE/.distr/services/msag/conf/rules || true
mkdir $WORKSPACE/.distr/services/msag/conf/rules/xsd || true
mkdir $WORKSPACE/.distr/webapps || true
mkdir $WORKSPACE/.distr/webapps/msag || true
mkdir $WORKSPACE/.distr/webapps/msag/rules || true
mkdir $WORKSPACE/.distr/webapps/msag/rules/rules || true
mkdir $WORKSPACE/.distr/webapps/msag/rules/rules/xsd || true
mkdir $WORKSPACE/.distr/webapps/msag/endpoints || true
mkdir $WORKSPACE/.distr/webapps/msag/endpoints/centers || true
mkdir $WORKSPACE/.distr/webapps/msag/endpoints/services || true
mkdir $WORKSPACE/.distr/webapps/msag/WEB-INF || true
mkdir $WORKSPACE/.distr/webapps/msag/WEB-INF/lib || true
mkdir $WORKSPACE/.distr/webapps/msag/WEB-INF/classes || true
mkdir $WORKSPACE/.distr/webapps/msag/WEB-INF/classes/locales || true

cp $WORKSPACE/.build/msag/bin/liblogger.so $WORKSPACE/.distr/lib
cp $WORKSPACE/.build/msag/bin/scag/msag $WORKSPACE/.distr/services/msag/bin
if [ "-$XERCES_HOME" != "-" ] ; then
cp $XERCES_HOME/lib/*.so $WORKSPACE/.distr/lib
fi
cp $WORKSPACE/config/templates/scag2/rules/xsd/bill.xsd $WORKSPACE/.distr/services/msag/conf/rules/xsd
cp $WORKSPACE/config/templates/scag2/rules/xsd/bill.xsd $WORKSPACE/.distr/webapps/msag/rules/rules/xsd
cp $WORKSPACE/config/scag2/smpp.dtd $WORKSPACE/.distr/services/msag/conf
cp $WORKSPACE/.build/msag-web/exploded/msag/endpoints/services/edit.jsp $WORKSPACE/.distr/webapps/msag/endpoints/services
cp $WORKSPACE/.build/msag-web/exploded/msag/endpoints/centers/edit.jsp $WORKSPACE/.distr/webapps/msag/endpoints/centers
cp $WORKSPACE/.build/msag-web/classes/msag/locales/* $WORKSPACE/.distr/webapps/msag/WEB-INF/classes/locales
cp $WORKSPACE/.build/msag-web/exploded/msag/WEB-INF/lib/scag.jar $WORKSPACE/.distr/webapps/msag/WEB-INF/lib
cp $WORKSPACE/.build/msag-web/jedit/jedit.jar $WORKSPACE/.distr/webapps/msag/rules/rules
cp $WORKSPACE/.build/msag-web/exploded/msag/rules/rules/jedit.jsp $WORKSPACE/.distr/webapps/msag/rules/rules
