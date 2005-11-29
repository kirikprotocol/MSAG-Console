package ru.novosoft.smsc.admin.resource_group;

import ru.novosoft.smsc.admin.AdminException;

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

	static {
		haname2sme.put("smscagent-harg", "SMSC");
		haname2sme.put("archiver-harg", "ArchiveDaemon");
		haname2sme.put("dbsme-harg", "dbSme");
		haname2sme.put("infosme-harg", "InfoSme");
		haname2sme.put("mcisme-harg", "MCISme");
		haname2sme.put("mciprof-harg", "MCIProf");
		haname2sme.put("inman-harg", "inman");
		haname2sme.put("proxysme-harg", "proxysme");

		smename2ha.put("SMSC", "smscagent-harg");
		smename2ha.put("ArchiveDaemon", "archiver-harg");
		smename2ha.put("dbSme", "dbsme-harg");
		smename2ha.put("InfoSme", "infosme-harg");
		smename2ha.put("MCISme", "mcisme-harg");
		smename2ha.put("MCIProf", "mciprof-harg");
		smename2ha.put("inman", "inman-harg");
		smename2ha.put("proxysme", "proxysme-harg");
   }

	public ResourceGroupHAImpl(String rgName)
	{
		super(NativeResourceGroupHA.new_ResourceGroup(rgName), true);
		name = (String) haname2sme.get(rgName);
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
			case STATUS_ONLINE:  NativeResourceGroupHA.ResourceGroup_online(swigCPtr); break;
			case STATUS_OFFLINE: NativeResourceGroupHA.ResourceGroup_offline(swigCPtr); break;
			default: throw new AdminException("Unknown online status");
		}
	}

	public byte getOnlineStatus(String nodeName)
	{
		if (NativeResourceGroupHA.ResourceGroup_onlineStatus(swigCPtr, nodeName))
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
