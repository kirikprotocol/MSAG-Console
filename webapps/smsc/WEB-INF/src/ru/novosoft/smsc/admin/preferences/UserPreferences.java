/*
 * Created by igork
 * Date: 05.11.2002
 * Time: 19:06:21
 */
package ru.novosoft.smsc.admin.preferences;

import ru.novosoft.smsc.jsp.util.tables.impl.*;

import java.util.Vector;
import java.util.Locale;

public class UserPreferences
{
	private int profilesPageSize = 20;
	private Vector profilesSortOrder = new Vector();
	private ProfileFilter profilesFilter = new ProfileFilter();

	private int aliasesPageSize = 20;
	private AliasFilter aliasesFilter = new AliasFilter();
	private Vector aliasesSortOrder = new Vector();

	private int subjectsPageSize = 20;
	private SubjectFilter subjectsFilter = new SubjectFilter();
	private Vector subjectsSortOrder = new Vector();

	private int usersPageSize = 20;
	private UserFilter userFilter = new UserFilter();
	private Vector usersSortOrder = new Vector();

	private int routesPageSize = 20;
	private RouteFilter routesFilter = new RouteFilter();
	private Vector routesSortOrder = new Vector();
	private boolean routeShowSrc = false;
	private boolean routeShowDst = false;

	private int smsviewPageSize = 20;
	private int smsviewMaxResults = 500;
	private String smsviewSortOrder = "date";

	private int localeResourcesPageSize = 20;
	private String localeResourcesSortOrder = "locale";

	private Locale locale = new Locale("ru");

	public UserPreferences()
	{
		this.profilesSortOrder.add("mask");
		this.aliasesSortOrder.add("Alias");
		this.subjectsSortOrder.add("Name");
		this.routesSortOrder.add("Route ID");
		this.usersSortOrder.add("login");
	}

	public int getProfilesPageSize()
	{
		return profilesPageSize;
	}

	public Vector getProfilesSortOrder()
	{
		return profilesSortOrder;
	}

	public ProfileFilter getProfilesFilter()
	{
		return profilesFilter;
	}

	public int getAliasesPageSize()
	{
		return aliasesPageSize;
	}

	public AliasFilter getAliasesFilter()
	{
		return aliasesFilter;
	}

	public Vector getAliasesSortOrder()
	{
		return aliasesSortOrder;
	}

	public int getSubjectsPageSize()
	{
		return subjectsPageSize;
	}

	public SubjectFilter getSubjectsFilter()
	{
		return subjectsFilter;
	}

	public Vector getSubjectsSortOrder()
	{
		return subjectsSortOrder;
	}

	public int getRoutesPageSize()
	{
		return routesPageSize;
	}

	public RouteFilter getRoutesFilter()
	{
		return routesFilter;
	}

	public Vector getRoutesSortOrder()
	{
		return routesSortOrder;
	}

	public boolean isRouteShowSrc()
	{
		return routeShowSrc;
	}

	public void setRouteShowSrc(boolean routeShowSrc)
	{
		this.routeShowSrc = routeShowSrc;
	}

	public boolean isRouteShowDst()
	{
		return routeShowDst;
	}

	public void setRouteShowDst(boolean routeShowDst)
	{
		this.routeShowDst = routeShowDst;
	}

	public int getUsersPageSize()
	{
		return usersPageSize;
	}

	public UserFilter getUserFilter()
	{
		return userFilter;
	}

	public Vector getUsersSortOrder()
	{
		return usersSortOrder;
	}

	public int getSmsviewPageSize()
	{
		return smsviewPageSize;
	}

	public void setSmsviewPageSize(int smsviewPageSize)
	{
		this.smsviewPageSize = smsviewPageSize;
	}

	public int getSmsviewMaxResults()
	{
		return smsviewMaxResults;
	}

	public void setSmsviewMaxResults(int smsviewMaxResults)
	{
		this.smsviewMaxResults = smsviewMaxResults;
	}

	public String getSmsviewSortOrder()
	{
		return smsviewSortOrder;
	}

	public void setSmsviewSortOrder(String smsviewSortOrder)
	{
		this.smsviewSortOrder = smsviewSortOrder;
	}

	public Locale getLocale()
	{
		return locale;
	}

	public int getLocaleResourcesPageSize()
	{
		return localeResourcesPageSize;
	}

	public void setLocaleResourcesPageSize(int localeResourcesPageSize)
	{
		this.localeResourcesPageSize = localeResourcesPageSize;
	}

	public String getLocaleResourcesSortOrder()
	{
		return localeResourcesSortOrder;
	}

	public void setLocaleResourcesSortOrder(String localeResourcesSortOrder)
	{
		this.localeResourcesSortOrder = localeResourcesSortOrder;
	}
}
