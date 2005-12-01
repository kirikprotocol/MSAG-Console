package ru.novosoft.smsc.admin.resource_group;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;

import java.util.HashMap;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 24.11.2005
 * Time: 17:20:10
 * To change this template use File | Settings | File Templates.
 */
public class ResourceGroupHAImpl extends ResourceGroupImpl
{
	protected static HashMap haname2sme = new HashMap();
	protected static HashMap smename2ha = new HashMap();

	static
	{
		haname2sme.put("smscagent-harg", "SMSC");
		haname2sme.put("archiver-harg", "ArchiveDaemon");
		haname2sme.put("dbsme-harg", "dbSme");
		haname2sme.put("infosme-harg", "InfoSme");
		haname2sme.put("mcisme-harg", "MCISme");
		haname2sme.put("mciprof-harg", "MCIProf");
		haname2sme.put("inman-harg", "inman");
		haname2sme.put("proxysme-harg", "proxysme");
		haname2sme.put("smsc-sarg", "SMSCservice");
		haname2sme.put("webapp-harg", "webapp");

		smename2ha.put("SMSC", "smscagent-harg");
		smename2ha.put("ArchiveDaemon", "archiver-harg");
		smename2ha.put("dbSme", "dbsme-harg");
		smename2ha.put("InfoSme", "infosme-harg");
		smename2ha.put("MCISme", "mcisme-harg");
		smename2ha.put("MCIProf", "mciprof-harg");
		smename2ha.put("inman", "inman-harg");
		smename2ha.put("proxysme", "proxysme-harg");
		smename2ha.put("SMSCservice", "smsc-sarg");
		smename2ha.put("webapp", "webapp-harg");
	}

	protected long swigCPtr;
	protected boolean swigCMemOwn;

	protected ResourceGroupHAImpl(long cPtr, boolean cMemoryOwn) {
		swigCMemOwn = cMemoryOwn;
		swigCPtr = cPtr;
	}

	public ResourceGroupHAImpl(String rgName) throws AdminException
	{
		this(NativeResourceGroupHA.new_ResourceGroup(rgName), true);
		name = (String) haname2sme.get(rgName);
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

	public void setOnlineStatus(byte onlineStatus) throws AdminException
	{
		switch (onlineStatus)
		{
			case ServiceInfo.STATUS_ONLINE:  NativeResourceGroupHA.ResourceGroup_online(swigCPtr); break;
			case ServiceInfo.STATUS_OFFLINE: NativeResourceGroupHA.ResourceGroup_offline(swigCPtr); break;
			default: throw new AdminException("Unknown online status");
		}
	}

	public byte getOnlineStatus(String nodeName)
	{
		logger.debug("Getting online status for rg: " + getName() + " at node: " + nodeName);
		if (NativeResourceGroupHA.ResourceGroup_onlineStatus(swigCPtr, nodeName))
			{
				logger.debug("ONLINE returned");
				return ServiceInfo.STATUS_ONLINE;
			}
			else
			{
				logger.debug("OFFLINE returned");
				return ServiceInfo.STATUS_OFFLINE;
			}
	}

	public byte getOnlineStatus()
	{
		byte result = ServiceInfo.STATUS_OFFLINE;
		String[] rgNodes = listNodes();
		for (int i = 0; i < rgNodes.length; i++)
		{
			if (getOnlineStatus(rgNodes[i]) == ServiceInfo.STATUS_ONLINE)
				{result = ServiceInfo.STATUS_ONLINE;break;}
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
