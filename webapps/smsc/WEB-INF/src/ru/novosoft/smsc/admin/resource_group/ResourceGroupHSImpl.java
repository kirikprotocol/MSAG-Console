package ru.novosoft.smsc.admin.resource_group;

import ru.novosoft.smsc.admin.AdminException;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 24.11.2005
 * Time: 16:05:35
 * To change this template use File | Settings | File Templates.
 */
public class ResourceGroupHSImpl  extends ResourceGroupImpl
{
	public ResourceGroupHSImpl(String rgName)
	{
		super(NativeResourceGroupHS.new_ResourceGroup(rgName), true);
	}

	public void switchOver(String nodeName)
	{
		NativeResourceGroupHS.ResourceGroup_switchOver(swigCPtr,nodeName);
	}

	public String[] listNodes()
	{
		return NativeResourceGroupHS.ResourceGroup_listNodes(swigCPtr);
	}

	public void setOnlineStatus(byte onlineStatus) throws AdminException
	{
		switch (onlineStatus)
		{
			case STATUS_ONLINE:  NativeResourceGroupHS.ResourceGroup_online(swigCPtr); break;
			case STATUS_OFFLINE: NativeResourceGroupHS.ResourceGroup_offline(swigCPtr); break;
			default: throw new AdminException("Unknown online status");
		}
	}

	public byte getOnlineStatus(String nodeName)
	{
		if (NativeResourceGroupHS.ResourceGroup_onlineStatus(swigCPtr, nodeName))
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

