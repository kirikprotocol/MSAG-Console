#include "CommandGetConfig.h"

namespace smsc {
namespace admin {
namespace protocol {

CommandGetConfig::CommandGetConfig(DOM_Document doc)
{
	setId(get_config);
	setData(doc);
}

}
}
}

