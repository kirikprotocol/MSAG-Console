package ru.novosoft.smsc.admin.resource_group;

import ru.novosoft.smsc.admin.AdminException;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 06.09.2005
 * Time: 15:53:06
 * To change this template use File | Settings | File Templates.
 */
public class ResourceGroupSingleImpl extends ResourceGroupImpl
{
	public ResourceGroupSingleImpl(String rgName)
	{
		super(NativeResourceGroupSingle.new_ResourceGroup(rgName), true);
	}

	public void switchOver(String nodeName)
	{
		NativeResourceGroupSingle.ResourceGroup_switchOver(swigCPtr,nodeName);
	}

	public String[] listNodes()
	{
		return NativeResourceGroupSingle.ResourceGroup_listNodes(swigCPtr);
	}

	public void setOnlineStatus(byte onlineStatus) throws AdminException
	{
		switch (onlineStatus)
		{
			case STATUS_ONLINE:  NativeResourceGroupSingle.ResourceGroup_online(swigCPtr); break;
			case STATUS_OFFLINE: NativeResourceGroupSingle.ResourceGroup_offline(swigCPtr); break;
			default: throw new AdminException("Unknown online status");
		}
	}

	public byte getOnlineStatus(String nodeName)
	{
		if (NativeResourceGroupSingle.ResourceGroup_onlineStatus(swigCPtr, nodeName))
			{return STATUS_ONLINE;}
			else return STATUS_OFFLINE;
	}

	public void offline()
	{
		NativeResourceGroupHA.ResourceGroup_offline(swigCPtr);
	}

	public void online()
	{
		NativeResourceGroupHA.ResourceGroup_online(swigCPtr);
	}

	public void delete()
	{
		NativeResourceGroupHA.delete_ResourceGroup(swigCPtr);
	}
}
