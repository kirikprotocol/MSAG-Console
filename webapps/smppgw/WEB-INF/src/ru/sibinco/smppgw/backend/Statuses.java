package ru.sibinco.smppgw.backend;


/**
 * Created by igork Date: 17.06.2004 Time: 18:24:03
 */
public class Statuses
{
  private boolean configChanged = false;
  private boolean routesChanged = false;
  private boolean usersChanged = false;
  private boolean smscsChanged = false;
  private boolean providersChanged = false;

  public boolean isConfigChanged()
  {
    return configChanged;
  }

  public void setConfigChanged(final boolean configChanged)
  {
    this.configChanged = configChanged;
  }

  public boolean isRoutesChanged()
  {
    return routesChanged;
  }

  public void setRoutesChanged(final boolean routesChanged)
  {
    this.routesChanged = routesChanged;
  }

  public boolean isUsersChanged()
  {
    return usersChanged;
  }

  public void setUsersChanged(boolean usersChanged)
  {
    this.usersChanged = usersChanged;
  }

  public boolean isSmscsChanged()
  {
    return smscsChanged;
  }

  public void setSmscsChanged(boolean smscsChanged)
  {
    this.smscsChanged = smscsChanged;
  }

  public boolean isProvidersChanged()
  {
    return providersChanged;
  }

  public void setProvidersChanged(boolean providersChanged)
  {
    this.providersChanged = providersChanged;
  }
}
