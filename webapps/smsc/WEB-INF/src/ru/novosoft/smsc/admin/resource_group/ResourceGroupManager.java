package ru.novosoft.smsc.admin.resource_group;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import org.apache.log4j.Category;

import java.util.*;
import java.io.IOException;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 06.09.2005
 * Time: 14:55:33
 * To change this template use File | Settings | File Templates.
 */
public class ResourceGroupManager
{
	private ResourceGroupList resGroups = new ResourceGroupList();
	private SMSCAppContext appContext;
	private Category logger = Category.getInstance(this.getClass());
	private Config config = null;

	public ResourceGroupManager(SMSCAppContext appContext) throws AdminException
	{
		this.appContext = appContext;
		this.config = this.appContext.getConfig();
		logger.debug("Initializing resource group manager");
    try {
      ResourceGroupNameMap.init(appContext.getConfig().getString(ResourceGroupConstants.RESOURCEGROUP_RG_MAP_FILE));
    } catch (Exception e) {
      logger.warn("Could not init RG mapping", e);
    }
    switch(getInstallType())
    {
      case ResourceGroupConstants.RESOURCEGROUP_TYPE_SINGLE:
/*				NativeResourceGroupSingle.LoadLibrary();
				logger.debug("JNI Library loaded");
				rgNames = NativeResourceGroupSingle.ResourceGroup_listGroups();
				for (int i = 0; i < rgNames.length; i++)
				{
					ResourceGroup rg = new ResourceGroupSingleImpl(rgNames[i]);
					add(rg);
					logger.debug("Resource group \"" + rg.getName() + "\" added");
				}*/
        break;
      case ResourceGroupConstants.RESOURCEGROUP_TYPE_HS:
/*				NativeResourceGroupHS.LoadLibrary();
				logger.debug("JNI Library loaded");
				rgNames = NativeResourceGroupHS.ResourceGroup_listGroups();
				for (int i = 0; i < rgNames.length; i++)
				{
					ResourceGroup rg = new ResourceGroupHSImpl(rgNames[i]);
					add(rg);
					logger.debug("Resource group \"" + rg.getName() + "\" added");
				}*/
        break;
      case ResourceGroupConstants.RESOURCEGROUP_TYPE_HA:
        NativeResourceGroupHA.LoadLibrary();
        logger.debug("JNI Library loaded");
        refreshResGroupList();
        break;
      default: throw new AdminException("Invalid type of installation");
    }
		logger.debug("Resource group manager initialized");
	}

	public void add(ResourceGroup rg) throws AdminException
	{
		resGroups.add(rg);
	}

	public ResourceGroup get(String name) throws AdminException
	{
		return resGroups.get(name);
	}

	public ResourceGroup remove(String name) throws AdminException
	{
		final ResourceGroup rg = get(name);
		resGroups.remove(name);
		return rg;
	}

	public List getResourceGroupsNames()
	{
		return resGroups.getResourceGroupsNames();
	}

	public int getResourceGroupsCount()
	{
		return resGroups.getResourceGroupsNames().size();
	}

	public void save()
	{
	}

	public byte getInstallType()
	{
		return appContext.getInstallType();
	}

	public boolean contains(String serviceId)
	{
		return resGroups.contains(serviceId);
	}

	public Map refreshServices(SmeManager smeManager) throws AdminException
	{
		Map result = new HashMap();
		for (Iterator i = resGroups.iterator(); i.hasNext();)
		{
      ResourceGroup rg = (ResourceGroup)i.next();
      try {
        if (rg.getName().equals(Constants.SMSC_SME_ID)) continue;
        ServiceInfo info = new ServiceInfo(rg.getName(), smeManager, rg.getOnlineStatus());
        result.put(info.getId(),info);
      } catch (AdminException e) {
        logger.warn("Couldn't get info for sme "+rg.getName());
      }
    }
		return result;
	}

	public void refreshResGroupList() throws AdminException
	{
		String[] rgNames;
		logger.debug("ResourceGroup_listGroups calling");
		rgNames = NativeResourceGroupHA.ResourceGroup_listGroups();
		logger.debug("ResourceGroup_listGroups returned resource groups:" + rgNames.length);
		for (int i = 0; i < rgNames.length; i++)
		{
			try
			{
				ResourceGroup rg = new ResourceGroupHAImpl(rgNames[i]);
				add(rg);
				logger.debug("Resource group \"" + rg.getName() + "\" added");
			}
			catch (AdminException e)
			{
				logger.error("Couldn't init resource group:" + rgNames[i], e);
			}
		}
	}
}
