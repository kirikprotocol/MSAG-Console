/*
 * Created by igork
 * Date: 05.11.2002
 * Time: 19:06:21
 */
package ru.novosoft.smsc.admin.preferences;

import ru.novosoft.smsc.jsp.util.tables.impl.ProfileFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.AliasFilter;

import java.util.Vector;

public class UserPreferences
{
	private int profilesPageSize = 20;
	private Vector profilesSortOrder = new Vector();
	private ProfileFilter profilesFilter = new ProfileFilter();

	private int aliasesPageSize = 20;
	private AliasFilter aliasesFilter = new AliasFilter();
	private Vector aliasesSortOrder = new Vector();

	public UserPreferences()
	{
		this.profilesSortOrder.add("mask");
		this.aliasesSortOrder.add("Alias");
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
}
