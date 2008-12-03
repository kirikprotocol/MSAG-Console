TEMPLATE_PROPS=msag.properties.niagara.tst

# Uncomment it to create usual build
# Comment it to create date separated build (to track external installations)
BUILD_DIR=/opt/home/msag_test/msag_build

# Change it for building updates for external installations
# DISTR_DIR=/opt/home/msag_test/distribs/msag.up
# Change it for building direct local updates
DISTR_DIR=/opt/home/msag_test/msag_distr
# Change it for building external installations
# DISTR_DIR=/opt/home/msag_test/distribs/msag

# Change it for building updates for external installations
# CATALINA_HOME=/opt/home/msag_test/distribs/msag.up/tomcat
# CATALINA_BASE=/opt/home/msag_test/distribs/msag.up
# Change it for building direct local updates
CATALINA_HOME=/opt/home/msag_test/msag_distr/tomcat
CATALINA_BASE=/opt/home/msag_test/msag_distr
# Change it for building external installations
# CATALINA_HOME=/opt/home/msag_test/distribs/msag/tomcat
# CATALINA_BASE=/opt/home/msag_test/distribs/msag

SMSC_DIR=/opt/home/msag_test/cvs/smsc
MAKE_ARGS=CXX=CC DEBUG=YES VERBOSE=YES TRACE=YES DISABLE_SMPP_CHECKS=YES UNICODE=YES MSAG_FAKE_STAT=NO MSAG_INMAN_BILL=NO LEAKTRACE=NO LOGGER_TIMED_CONFIG_RELOAD=NO USECOLORS=YES XTARGET="-xtarget=ultra3 -xarch=v9 -xmemalign=8i -xcode=pic32" B64=YES

