#include "CommandGetMonitoringData.h"

namespace smsc {
namespace admin {
namespace protocol {

CommandGetMonitoringData::CommandGetMonitoringData(DOM_Document doc)
{
	setId(get_monitoring);
	setData(doc);
}

}
}
}

