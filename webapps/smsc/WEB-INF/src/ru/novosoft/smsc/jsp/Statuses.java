/*
 * Created by igork
 * Date: 13.11.2002
 * Time: 16:37:50
 */
package ru.novosoft.smsc.jsp;

public interface Statuses
{
	boolean isRoutesChanged();

	void setRoutesChanged(boolean routesChanged);

	boolean isSubjectsChanged();

	void setSubjectsChanged(boolean subjectsChanged);

	boolean isAliasesChanged();

	void setAliasesChanged(boolean aliasesChanged);

	boolean isProfilesChanged();

	void setProfilesChanged(boolean profilesChanged);

	boolean isHostsChanged();

	void setHostsChanged(boolean hostsChanged);

	boolean isServicesChanged();

	void setServicesChanged(boolean servicesChanged);

	boolean isUsersChanged();

	void setUsersChanged(boolean usersChanged);

	boolean isSmscChanged();

	void setSmscChanged(boolean smscChanged);
}
