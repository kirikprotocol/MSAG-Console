package ru.novosoft.smsc.jsp;

/*
 * Created by igork
 * Date: 13.11.2002
 * Time: 16:37:50
 */

public interface Statuses
{
  boolean isRoutesChanged();

  void setRoutesChanged(boolean routesChanged);

  boolean isSubjectsChanged();

  void setSubjectsChanged(boolean subjectsChanged);

  boolean isRoutesSaved();

  void setRoutesSaved(boolean routesSaved);

  public boolean isRoutesRestored();

  public void setRoutesRestored(boolean routesRestored);

  public boolean isRoutesLoaded();

  public void setRoutesLoaded(boolean routesLoaded);

  boolean isAliasesChanged();

  void setAliasesChanged(boolean aliasesChanged);

  boolean isProfilesChanged();

  void setProfilesChanged(boolean profilesChanged);

  boolean isHostsChanged();

  void setHostsChanged(boolean hostsChanged);

/*
  boolean isServicesChanged();
	void setServicesChanged(boolean servicesChanged);
*/
  boolean isUsersChanged();

  void setUsersChanged(boolean usersChanged);

  boolean isSmscChanged();

  void setSmscChanged(boolean smscChanged);

  boolean isWebXmlChanged();

  void setWebXmlChanged(boolean webXmlChanged);

  boolean isProvidersChanged();

  void setProvidersChanged(boolean providersChanged);

  boolean isCategoriesChanged();

  void setCategoriesChanged(boolean providersChanged);
}
