/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 18:49:34
 */
package ru.novosoft.smsc.jsp.smsc.profiles;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.util.tables.NullResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.ProfileQuery;

import java.util.List;

public class Index extends IndexBean
{
	public static final int RESULT_ADD = IndexBean.PRIVATE_RESULT;
	public static final int RESULT_EDIT = IndexBean.PRIVATE_RESULT + 1;
	protected static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 2;

	protected QueryResultSet profiles = null;

	protected String profileMask = null;

	protected String mbAdd = null;
	protected String mbEdit = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		pageSize = preferences.getProfilesPageSize();
		if (sort != null)
			preferences.getProfilesSortOrder().set(0, sort);
		else
			sort = (String) preferences.getProfilesSortOrder().get(0);

		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors)
	{
		profiles = new NullResultSet();

		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbAdd != null)
			return RESULT_ADD;
		else if (mbEdit != null)
			return RESULT_EDIT;

		try
		{
			logger.debug("Profiles.Index - process with sorting [" + (String) preferences.getProfilesSortOrder().get(0) + "]");
			profiles = smsc.queryProfiles(new ProfileQuery(pageSize, preferences.getProfilesFilter(), preferences.getProfilesSortOrder(), startPosition));
			totalSize = profiles.getTotalSize();
		}
		catch (AdminException e)
		{
			logger.error("Couldn't query profiles", e);
			return error(SMSCErrors.error.profiles.queryError, e);
		}

		return isEditAllowed() ? RESULT_OK : error(SMSCErrors.error.profiles.smscNotConnected);
	}

	public QueryResultSet getProfiles()
	{
		return profiles;
	}

	/******************** properties *************************/

	public String getMbAdd()
	{
		return mbAdd;
	}

	public void setMbAdd(String mbAdd)
	{
		this.mbAdd = mbAdd;
	}

	public String getProfileMask()
	{
		return profileMask;
	}

	public void setProfileMask(String profileMask)
	{
		this.profileMask = profileMask;
	}

	public String getMbEdit()
	{
		return mbEdit;
	}

	public void setMbEdit(String mbEdit)
	{
		this.mbEdit = mbEdit;
	}
	public boolean isEditAllowed()
	{
		return smsc.getInfo().getStatus() == ServiceInfo.STATUS_RUNNING;
	}
}
