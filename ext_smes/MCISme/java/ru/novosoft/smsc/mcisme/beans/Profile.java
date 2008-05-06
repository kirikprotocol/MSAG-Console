package ru.novosoft.smsc.mcisme.beans;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.mcisme.backend.ProfileInfo;
import ru.novosoft.smsc.mcisme.backend.ProfStorageAccessor;

import javax.servlet.http.HttpServletRequest;
import java.util.*;
import java.io.*;

/**
 * User: miker
 * Date: 01.09.2006
 */
public class Profile extends MCISmeBean
{
	private final static String MCI_PROF_STORAGE_FILE = "profstorage.properties";
	ProfStorageAccessor	storage = null;

	private String mbQuery = null;
	private String mbSet = null;

	private String Address = null;
//	private boolean Inform = false;
//	private boolean Notify = false;
//	private int InformSelectedTemplate = 2;
//	private int NotifySelectedTemplate = 2;
//	private boolean Busy = false;
//	private boolean NoReplay = false;
//	private boolean Absent = false;
//	private boolean Detach = false;
//	private boolean Uncond = false;
//	private boolean Other = false;
	ProfileInfo	profile = new ProfileInfo();

	public class Identity 
	{
		public int    id = 0;
		public String name = "";

		public Identity() {}
		public Identity(int id, String name) 
		{
			this.id = id; this.name = name;
		}
	}
	private class IdentityComparator implements Comparator
	{
		public int compare(Object o1, Object o2) 
		{
			if (o1 != null && o1 instanceof Identity && o2 != null && o2 instanceof Identity) 
			{
				Identity i1 = (Identity)o1; Identity i2 = (Identity)o2;
				return (i1.id-i2.id);
			}
			return 0;
		}
	}
	private SortedList informTemplates = new SortedList(new IdentityComparator());
	private SortedList notifyTemplates = new SortedList(new IdentityComparator());

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)  return result;
		int storageResult = initProfSorageAccessor(getConfig());
		int loadResult = loadupFromConfig(getConfig());

		return result;
	}
	public int process(HttpServletRequest request)
	{
		int result = super.process(request);
		if (result != RESULT_OK)  return result;

		if(mbQuery != null)	return onQuery();
		if(mbSet != null) return onSet();

		return result;
	}
	private int loadupFromConfig(Config config)
	{
		try 
		{
//			defaultInformId = config.getInt(INFORM_TEMPLATES_SECTION_NAME+".default");
			Set set = config.getSectionChildShortSectionNames(INFORM_TEMPLATES_SECTION_NAME);
			for (Iterator i = set.iterator(); i.hasNext();) 
			{
				String section = (String)i.next();
				int id = config.getInt(INFORM_TEMPLATES_SECTION_NAME+'.'+section+".id");
				informTemplates.add(new Identity(id, section));
			}
		} 
		catch (Exception e) 
		{
			return error("mcisme.error.template_load_i", e);
		}
    
		try 
		{
//			defaultNotifyId = config.getInt(NOTIFY_TEMPLATES_SECTION_NAME+".default");
			Set set = config.getSectionChildShortSectionNames(NOTIFY_TEMPLATES_SECTION_NAME);
			for (Iterator i = set.iterator(); i.hasNext();) 
			{
				String section = (String)i.next();
				int id = config.getInt(NOTIFY_TEMPLATES_SECTION_NAME+'.'+section+".id");
				notifyTemplates.add(new Identity(id, section));
			}
		} 
		catch (Exception e) 
		{
			return error("mcisme.error.template_load_n", e);
		}

		return RESULT_OK;
	}
	private int initProfSorageAccessor(Config config)
	{
		try 
		{
			String host = config.getString(PROFSTORAGE_SECTION_NAME+".host");
			int port = config.getInt(PROFSTORAGE_SECTION_NAME+".port");
			storage = new ProfStorageAccessor(host, port);
		} 
		catch (Exception e) 
		{
			return error("mcisme.error.profile_storage", e);
		}
		return RESULT_OK;
	}
	private int onQuery()
	{
		if(!checkAddress())
		{
			Address = null;
			return error("mcisme.error.bad_address");
		}
		if(null != storage)
			if(1 == storage.getProfile(Address, profile))
				return error("Connection to Profile Storage failed.");
		if(profile.isAddressNotExists)
			warning("Profile for this Address is not exists (using default). Make sure that Address is correct.");

		return 0;
	}
	private int onSet()
	{
		if(!checkAddress())
		{
			Address = null;
			return error("mcisme.error.bad_address");
		}
		if(null != storage)
			if(1 == storage.setProfile(Address, profile))
				return error("Connection to Profile Storage failed.");

		return 0;
	}
	private boolean checkAddress()
	{
		return true;
	}
	public String getMbQuery() 
	{
		return mbQuery;
	}
	public void setMbQuery(String mbQuery) 
	{
		this.mbQuery = mbQuery;
	}
	public String getMbSet() 
	{
		return mbSet;
	}
	public void setMbSet(String mbSet) 
	{
		this.mbSet = mbSet;
	}
	public String getAddress() 
	{
		return Address;
	}
	public void setAddress(String value) 
	{
		this.Address = value;
	}
	public boolean getInform()
	{
		return profile.inform;
	}
	public void setInform(boolean value)
	{
		this.profile.inform = value;
	}
	public boolean getNotify()
	{
		return profile.notify;
	}
	public void setNotify(boolean value)
	{
		this.profile.notify = value;
	}
  public boolean getWantNotifyMe()
  {
    return profile.wantNotifyMe;
  }
  public void setWantNotifyMe(boolean value)
  {
    this.profile.wantNotifyMe = value;
  }
	public List getInformTemplates()
	{
		return informTemplates;
	}
	public int getSelectedInformId() 
	{
		return profile.informSelectedTemplate;
	}
	public void setInformTemplate(String value) 
	{
		try { this.profile.informSelectedTemplate = Integer.decode(value).intValue(); }
		catch (NumberFormatException e) 
		{
			logger.debug("Invalid int MCISme.Reasons.strategy parameter value: \"" + value + '"', e);
		}
	}
	public List getNotifyTemplates() 
	{
		return notifyTemplates;
	}
	public int getSelectedNotifyId() 
	{
		return profile.notifySelectedTemplate;
	}
	public void setNotifyTemplate(String value) 
	{
		try { this.profile.notifySelectedTemplate = Integer.decode(value).intValue(); }
		catch (NumberFormatException e) 
		{
			logger.debug("Invalid int MCISme.Reasons.strategy parameter value: \"" + value + '"', e);
		}
	}
	public boolean getBusy()
	{
		return profile.busy;
	}
	public void setBusy(boolean value)
	{
		this.profile.busy = value;
	}
	public boolean getAbsent()
	{
		return profile.absent;
	}
	public void setAbsent(boolean value)
	{
		this.profile.absent = value;
	}
	public boolean getNoReplay()
	{
		return profile.noReplay;
	}
	public void setNoReplay(boolean value)
	{
		this.profile.noReplay = value;
	}
	public boolean getDetach()
	{
		return profile.detach;
	}
	public void setDetach(boolean value)
	{
		this.profile.detach = value;
	}
	public boolean getUncond()
	{
		return profile.uncond;
	}
	public void setUncond(boolean value)
	{
		this.profile.uncond = value;
	}
	public boolean getOther()
	{
		return profile.other;
	}
	public void setOther(boolean value)
	{
		this.profile.other = value;
	}
	public boolean isAddressNotExists()
	{
		return profile.isAddressNotExists;
	}

}
