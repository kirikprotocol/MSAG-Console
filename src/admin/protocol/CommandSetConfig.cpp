#include "CommandSetConfig.h"

namespace smsc {
namespace admin {
namespace protocol {

CommandSetConfig::CommandSetConfig(DOM_Document doc)
{
	setId(set_config);
	setData(doc);
}

}
}
}

