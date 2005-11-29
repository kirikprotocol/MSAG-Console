package ru.novosoft.smsc.admin.resource_group;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.AdminException;

import java.util.Map;
import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 06.09.2005
 * Time: 14:55:17
 * To change this template use File | Settings | File Templates.
 */
public interface ResourceGroup
{
	final byte STATUS_UNKNOWN = 0;
	final byte STATUS_ONLINE = 1;
	final byte STATUS_OFFLINE = 2;

	String getName();

	void setOnlineStatus(byte Status) throws AdminException;

	byte getOnlineStatus(String nodeName);

	void switchOver(String nodeName);

	void delete();

	void online();

	void offline();

	String[] listNodes();
}
