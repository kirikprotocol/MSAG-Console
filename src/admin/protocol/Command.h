#ifndef SMSC_ADMIN_PROTOCOL_COMMAND
#define SMSC_ADMIN_PROTOCOL_COMMAND

#include <inttypes.h>
#include <xercesc/dom/DOM_Document.hpp>
#include <log4cpp/Category.hh>

#include <admin/AdminException.h>

using smsc::admin::AdminException;

namespace smsc {
namespace admin {
namespace protocol {

class smsc::admin::protocol::CommandReader;

class Command {
	friend class smsc::admin::protocol::CommandReader;

public:
	enum Id
	{
		undefined,
		get_config,
		set_config,
		get_logs,
		get_monitoring,
		start_service,
		shutdown_service,
		kill_service,
		add_service,
		remove_service,
		list_services
	};
	
	virtual ~Command();
	
	const Id getId() const             {return id;}
	const char * const getName() const {return names[id].name;}
	const DOM_Document getData() const {return data;}
	
protected:
	struct _Command {
		char * name;
		Id id;
	};
	
	Command();
	Id id;
	DOM_Document data;
	log4cpp::Category &logger;
	static const uint8_t commands_quantity = 11;
	static const _Command names[commands_quantity];
	
	void setId(Id newId)  {id = newId;}
	static Id getCommandIdByName(const char * const name);
	void setData(DOM_Document newData) {data = newData;}

private:
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND
