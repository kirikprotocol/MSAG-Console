/**
 * Created by igork
 * Date: Nov 29, 2002
 * Time: 7:02:44 PM
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.DataItem;

public class UserFilter implements Filter
{
	private String loginFilter = "";
	private String firstNameFilter = "";
	private String lastNameFilter = "";
	private String departmentFilter = "";

	private boolean isStringEmpty(String s)
	{
		return s == null || s.length() == 0;
	}

	public boolean isEmpty()
	{
		return isStringEmpty(loginFilter) && isStringEmpty(firstNameFilter) && isStringEmpty(lastNameFilter) && isStringEmpty(departmentFilter);
	}

	public boolean isItemAllowed(DataItem item)
	{
		UserDataItem uditem = (UserDataItem) item;
		return (isStringEmpty(loginFilter) || uditem.getLogin().matches(loginFilter))
				&& (isStringEmpty(firstNameFilter) || uditem.getFirstName().matches(firstNameFilter))
				&& (isStringEmpty(lastNameFilter) || uditem.getLastName().matches(lastNameFilter))
				&& (isStringEmpty(departmentFilter) || uditem.getDept().matches(departmentFilter));
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
}
