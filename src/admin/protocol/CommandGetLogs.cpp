#include "CommandGetLogs.h"

namespace smsc {
namespace admin {
namespace protocol {

CommandGetLogs::CommandGetLogs(DOM_Document doc)
{
	setId(get_logs);
	setData(doc);
}

}
}
}

