package ru.novosoft.smsc.jsp.smsc.users;

import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.jsp.util.tables.impl.UserFilter;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.util.List;

/**
 * Created by igork
 * Date: Feb 10, 2003
 * Time: 6:37:10 PM
 */
public class UsersFilter extends SmscBean
{
	private UserFilter filter = null;

	private String loginFilter = null;
	private String firstNameFilter = null;
	private String lastNameFilter = null;
	private String departmentFilter = null;
	private String initialized = null;

	private String mbApply = null;
	private String mbCancel = null;


	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		filter = appContext.getUserPreferences().getUserFilter();

		if (initialized == null)
		{
			loginFilter = filter.getLoginFilter();
			firstNameFilter = filter.getFirstNameFilter();
			lastNameFilter = filter.getLastNameFilter();
			departmentFilter = filter.getDepartmentFilter();
		}
		if (loginFilter == null)
			loginFilter = "";
		if (firstNameFilter == null)
			firstNameFilter = "";
		if (lastNameFilter == null)
			lastNameFilter = "";
		if (departmentFilter == null)
			departmentFilter = "";

		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbApply != null)
		{
			filter.setLoginFilter(loginFilter);
			filter.setFirstNameFilter(firstNameFilter);
			filter.setLastNameFilter(lastNameFilter);
			filter.setDepartmentFilter(departmentFilter);
			return RESULT_DONE;
		}
		else if (mbCancel != null)
			return RESULT_DONE;

		return RESULT_OK;
	}

	public UserFilter getFilter()
	{
		return filter;
	}

	public void setFilter(UserFilter filter)
	{
		this.filter = filter;
	}

	public String getLoginFilter()
	{
		return loginFilter;
	}

	public void setLoginFilter(String loginFilter)
	{
		this.loginFilter = loginFilter;
	}

	public String getFirstNameFilter()
	{
		return firstNameFilter;
	}

	public void setFirstNameFilter(String firstNameFilter)
	{
		this.firstNameFilter = firstNameFilter;
	}

	public String getLastNameFilter()
	{
		return lastNameFilter;
	}

	public void setLastNameFilter(String lastNameFilter)
	{
		this.lastNameFilter = lastNameFilter;
	}

	public String getDepartmentFilter()
	{
		return departmentFilter;
	}

	public void setDepartmentFilter(String departmentFilter)
	{
		this.departmentFilter = departmentFilter;
	}

	public String getMbApply()
	{
		return mbApply;
	}

	public void setMbApply(String mbApply)
	{
		this.mbApply = mbApply;
	}

	public String getMbCancel()
	{
		return mbCancel;
	}

	public void setMbCancel(String mbCancel)
	{
		this.mbCancel = mbCancel;
	}

	public String getInitialized()
	{
		return initialized;
	}

	public void setInitialized(String initialized)
	{
		this.initialized = initialized;
	}
}
