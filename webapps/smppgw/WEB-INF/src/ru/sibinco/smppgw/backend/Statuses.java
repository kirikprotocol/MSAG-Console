package ru.sibinco.smppgw.backend;


/**
 * Created by igork Date: 17.06.2004 Time: 18:24:03
 */
public class Statuses
{
  private boolean configChanged = false;
  private boolean routesChanged = false;

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
}
