/*
 * Created by igork
 * Date: 05.11.2002
 * Time: 19:06:21
 */
package ru.novosoft.smsc.admin.preferences;

import ru.novosoft.smsc.jsp.util.tables.impl.ProfileFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.AliasFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.SubjectFilter;

import java.util.Vector;

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

	public UserPreferences()
	{
		this.profilesSortOrder.add("mask");
		this.aliasesSortOrder.add("Alias");
		this.subjectsSortOrder.add("Name");
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
}
