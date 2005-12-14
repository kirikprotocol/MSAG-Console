package ru.novosoft.smsc.admin.resource_group;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 06.09.2005
 * Time: 14:55:17
 * To change this template use File | Settings | File Templates.
 */
public interface ResourceGroup
{
	String getName();

	byte getOnlineStatus();

	void switchOver(String nodeName);

	void delete();

	void online();

	void offline();

	String[] listNodes();
}
