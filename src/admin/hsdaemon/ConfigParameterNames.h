#ifndef SMSC_ADMIN_DAEMON_CONFIG_PARAMETER_NAMES
#define SMSC_ADMIN_DAEMON_CONFIG_PARAMETER_NAMES

namespace smsc {
namespace admin {
namespace hsdaemon {

static const char * const CONFIG_SERVICES_SECTION = "services.";
static const char * const CONFIG_HOME_PARAMETER = "admin.daemon.home";
static const char * const CONFIG_SERVICES_FOLDER_PARAMETER = "admin.daemon.services folder";
static const char * const CONFIG_HOST_PARAMETER = "admin.daemon.host";
static const char * const CONFIG_PORT_PARAMETER = "admin.daemon.port";
static const char * const CONFIG_STDERR_PARAMETER = "admin.daemon.stderr";
static const char * const CONFIG_LOGGER_SECTION = "admin.daemon.logger.";
static const char * const CONFIG_LOGGER_CONFIG_PARAMETER = "admin.daemon.logger.configFile";
static const char * const CONFIG_SHUTDOWN_TIMEOUT = "admin.daemon.shutdown timeout";

static const char * const CONFIG_ICON_HOST="admin.daemon.icon.host";
static const char * const CONFIG_ICON_PORT="admin.daemon.icon.port";
static const char * const CONFIG_ICON_OTHERHOST="admin.daemon.icon.otherhost";
static const char * const CONFIG_ICON_OTHERPORT="admin.daemon.icon.otherport";

/*static const char * const CONFIG_SERVICES_SECTION;
static const char * const CONFIG_HOME_PARAMETER;
static const char * const CONFIG_SERVICES_FOLDER_PARAMETER;
static const char * const CONFIG_HOST_PARAMETER;
static const char * const CONFIG_PORT_PARAMETER;
static const char * const CONFIG_STDERR_PARAMETER;
static const char * const CONFIG_LOGGER_SECTION;
static const char * const CONFIG_LOGGER_CONFIG_PARAMETER;*/

}
}
}

#endif //ifndef SMSC_ADMIN_DAEMON_CONFIG_PARAMETER_NAMES
