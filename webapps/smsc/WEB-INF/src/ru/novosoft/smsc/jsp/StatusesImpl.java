/*
 * Created by igork
 * Date: 13.11.2002
 * Time: 16:29:15
 */
package ru.novosoft.smsc.jsp;

public class StatusesImpl implements Statuses
{
	private boolean routesChanged = false;
  private boolean routesSaved = false;
  private boolean routesRestored = false;
  private boolean routesLoaded = false;
	private boolean subjectsChanged = false;
	private boolean aliasesChanged = false;
	private boolean profilesChanged = false;
	private boolean hostsChanged = false;
	//private boolean servicesChanged = false;
	private boolean usersChanged = false;
	private boolean smscChanged = false;
  private boolean webXmlChanged = false;

	public boolean isRoutesChanged() {
		return routesChanged;
	}
	public void setRoutesChanged(boolean routesChanged) {
		this.routesChanged = routesChanged;
    if (routesChanged) { routesRestored  = false;  routesLoaded = false; }
	}

  public boolean isSubjectsChanged() {
    return subjectsChanged;
  }
  public void setSubjectsChanged(boolean subjectsChanged) {
    this.subjectsChanged = subjectsChanged;
    if (subjectsChanged) { routesRestored  = false;  routesLoaded = false; }
  }

  public boolean isRoutesSaved() {
    return routesSaved;
  }
  public void setRoutesSaved(boolean routesSaved) {
    this.routesSaved = routesSaved;
  }
  public boolean isRoutesRestored() {
    return routesRestored;
  }
  public void setRoutesRestored(boolean routesRestored) {
    this.routesRestored = routesRestored;
  }

  public boolean isRoutesLoaded() {
    return routesLoaded;
  }
  public void setRoutesLoaded(boolean routesLoaded) {
    this.routesLoaded = routesLoaded;
  }

	public boolean isAliasesChanged()
	{
		return aliasesChanged;
	}
	public void setAliasesChanged(boolean aliasesChanged)
	{
		this.aliasesChanged = aliasesChanged;
	}

	public boolean isProfilesChanged()
	{
		return profilesChanged;
	}
	public void setProfilesChanged(boolean profilesChanged)
	{
		this.profilesChanged = profilesChanged;
	}

	public boolean isHostsChanged()
	{
		return hostsChanged;
	}
	public void setHostsChanged(boolean hostsChanged)
	{
		this.hostsChanged = hostsChanged;
	}

/*
	public boolean isServicesChanged()
	{
		return servicesChanged;
	}

	public void setServicesChanged(boolean servicesChanged)
	{
		this.servicesChanged = servicesChanged;
	}
*/

	public boolean isUsersChanged()
	{
		return usersChanged;
	}
	public void setUsersChanged(boolean usersChanged)
	{
		this.usersChanged = usersChanged;
	}

	public boolean isSmscChanged()
	{
		return smscChanged;
	}
	public void setSmscChanged(boolean smscChanged)
	{
		this.smscChanged = smscChanged;
	}

  public boolean isWebXmlChanged()
  {
    return webXmlChanged;
  }

  public void setWebXmlChanged(boolean webXmlChanged)
  {
    this.webXmlChanged = webXmlChanged;
  }
}
