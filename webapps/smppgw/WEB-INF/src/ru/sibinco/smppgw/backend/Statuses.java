package ru.sibinco.smppgw.backend;


/**
 * Created by igork Date: 17.06.2004 Time: 18:24:03
 */
public class Statuses
{
  private boolean configChanged = false;

  public boolean isConfigChanged()
  {
    return configChanged;
  }

  public void setConfigChanged(boolean configChanged)
  {
    this.configChanged = configChanged;
  }
}
