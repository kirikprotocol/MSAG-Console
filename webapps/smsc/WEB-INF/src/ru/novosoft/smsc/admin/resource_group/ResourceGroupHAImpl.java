package ru.novosoft.smsc.admin.resource_group;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.smsc_service.SmscList;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 24.11.2005
 * Time: 17:20:10
 * To change this template use File | Settings | File Templates.
 */
public class ResourceGroupHAImpl extends ResourceGroupImpl
{

	protected long swigCPtr;
	protected boolean swigCMemOwn;

	protected ResourceGroupHAImpl(long cPtr, boolean cMemoryOwn) {
		swigCMemOwn = cMemoryOwn;
		swigCPtr = cPtr;
	}

	public ResourceGroupHAImpl(String rgName) throws AdminException
	{
		this(NativeResourceGroupHA.new_ResourceGroup(rgName), true);
		name = ResourceGroupNameMap.getSmeByName(rgName);
		if (name == null) throw new AdminException("Couldn't map resource group ha-name:" + rgName);
		logger.info("Resource group (haname="+rgName+", smename="+name+") created");
	}

	public void switchOver(String nodeName)
	{
		NativeResourceGroupHA.ResourceGroup_switchOver(swigCPtr,nodeName);
	}

	public String[] listNodes()
	{
		return NativeResourceGroupHA.ResourceGroup_listNodes(swigCPtr);
	}

	public byte getOnlineStatus()
	{
		byte result = ServiceInfo.STATUS_OFFLINE;
		String[] rgNodes = listNodes();
		for (int i = 0; i < rgNodes.length; i++)
		{
			if (NativeResourceGroupHA.ResourceGroup_onlineStatus(swigCPtr, rgNodes[i]))
				return SmscList.getNodeId(rgNodes[i]);
		}
		return result;
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
