/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 18:49:34
 */
package ru.novosoft.smsc.jsp.smsc.profiles;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.SmscBean;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.ProfileFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.ProfileQuery;

import java.util.List;
import java.util.Vector;

public class Index extends SmscBean
{
	public static final int RESULT_FILTER = PRIVATE_RESULT;
	public static final int RESULT_ADD = PRIVATE_RESULT + 1;
	public static final int RESULT_EDIT = PRIVATE_RESULT + 2;

	protected QueryResultSet profiles = null;

	protected int startPosition = 0;
	protected int totalSize = 0;
	protected String profileMask = null;

	protected String mbAdd = null;
	protected String mbEdit = null;
	protected String mbFilter = null;
	protected String mbFirst = null;
	protected String mbPrev = null;
	protected String mbNext = null;
	protected String mbLast = null;

	protected String sort = null;


	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		UserPreferences up = appContext.getUserPreferences();
		int pageSize = up.getProfilesPageSize();
		if (sort != null)
			up.getProfilesSortOrder().set(0, sort);
		else
			sort = (String) up.getProfilesSortOrder().get(0);

		if (mbAdd != null)
			return RESULT_ADD;
		else if (mbEdit != null)
			return RESULT_EDIT;
		else if (mbFilter != null)
			return RESULT_FILTER;
		else if (mbFirst != null)
			startPosition = 0;
		else if (mbPrev != null)
			startPosition = startPosition > pageSize ? startPosition - pageSize : 0;
		else if (mbNext != null)
			startPosition += pageSize;
		else if (mbLast != null)
			startPosition = (totalSize / pageSize + (totalSize % pageSize == 0 ? -1 : 0)) * pageSize;

		try
		{
			logger.debug("Profiles.Index - process with sorting [" + (String)up.getProfilesSortOrder().get(0) + "]");
			profiles = smsc.queryProfiles(new ProfileQuery(pageSize, up.getProfilesFilter(), up.getProfilesSortOrder(), startPosition));
			totalSize = profiles.getTotalSize();
		}
		catch (AdminException e)
		{
			logger.error("Couldn't query profiles", e);
			return error(SMSCErrors.error.profiles.queryError, e);
		}

		return RESULT_OK;
	}

	public QueryResultSet getProfiles()
	{
		return profiles;
	}

	public boolean isFirst()
	{
		return startPosition == 0;
	}

	public boolean isLast()
	{
		return profiles.isLast();
	}

	/******************** properties *************************/

	public int getStartPosition()
	{
		return startPosition;
	}

	public void setStartPosition(int startPosition)
	{
		this.startPosition = startPosition;
	}

	public String getMbAdd()
	{
		return mbAdd;
	}

	public void setMbAdd(String mbAdd)
	{
		this.mbAdd = mbAdd;
	}

	public String getMbFilter()
	{
		return mbFilter;
	}

	public void setMbFilter(String mbFilter)
	{
		this.mbFilter = mbFilter;
	}

	public String getMbFirst()
	{
		return mbFirst;
	}

	public void setMbFirst(String mbFirst)
	{
		this.mbFirst = mbFirst;
	}

	public String getMbPrev()
	{
		return mbPrev;
	}

	public void setMbPrev(String mbPrev)
	{
		this.mbPrev = mbPrev;
	}

	public String getMbNext()
	{
		return mbNext;
	}

	public void setMbNext(String mbNext)
	{
		this.mbNext = mbNext;
	}

	public String getMbLast()
	{
		return mbLast;
	}

	public void setMbLast(String mbLast)
	{
		this.mbLast = mbLast;
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

	public int getTotalSize()
	{
		return totalSize;
	}

	public void setTotalSize(int totalSize)
	{
		this.totalSize = totalSize;
	}

	public String getSort()
	{
		return sort;
	}

	public void setSort(String sort)
	{
		this.sort = sort;
	}
}
