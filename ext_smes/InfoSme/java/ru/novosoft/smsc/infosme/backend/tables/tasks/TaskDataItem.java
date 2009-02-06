package ru.novosoft.smsc.infosme.backend.tables.tasks;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

import java.util.Date;

/**
 * Created by igork
 * Date: Sep 2, 2003
 * Time: 12:58:13 PM
 */
public class TaskDataItem extends AbstractDataItem
{
  public TaskDataItem(String id, String name, String provider, boolean enabled, int priority, String retryPolicy, boolean replaceMessage, String svcType, boolean generating, boolean processing, boolean trackIntegrity, Date startDate, Date endDate, String owner, boolean delivery)
  {
    values.put("id", id);
    values.put("name", name);
    values.put("provider", provider);
    values.put("enabled", Boolean.valueOf(enabled));
    values.put("priority", new Integer(priority));
    values.put("retryPolicy", retryPolicy);
    values.put("replaceMessage", Boolean.valueOf(replaceMessage));
    values.put("svcType", svcType);
    values.put("generating", Boolean.valueOf(generating));
    values.put("processing", Boolean.valueOf(processing));
    values.put("trackIntegrity", Boolean.valueOf(trackIntegrity));
    values.put("startDate", startDate);
    values.put("endDate", endDate);
    values.put("owner", owner);
    values.put("delivery", Boolean.valueOf(delivery));
  }

  public void setRetryPolicy(String policy) {
    values.put("retryPolicy", policy);
  }

  public String getRetryPolicy() {
    return (String)values.get("retryPolicy");
  }

  public String getId()
  {
    return (String) values.get("id");
  }

  public String getName()
  {
    return (String) values.get("name");
  }

  public String getProvider()
  {
    return (String) values.get("provider");
  }

  public boolean isEnabled()
  {
    return ((Boolean) values.get("enabled")).booleanValue();
  }

  public int getPriority()
  {
    return ((Integer) values.get("priority")).intValue();
  }

  public boolean isRetryOnFail()
  {
    return ((Boolean) values.get("retryOnFail")).booleanValue();
  }

  public boolean isReplaceMessage()
  {
    return ((Boolean) values.get("replaceMessage")).booleanValue();
  }

  public String getSvcType()
  {
    return (String) values.get("svcType");
  }

  public boolean isGenerating()
  {
    return ((Boolean) values.get("generating")).booleanValue();
  }

  public boolean isProcessing()
  {
    return ((Boolean) values.get("processing")).booleanValue();
  }

  public boolean isTrackIntegrity()
  {
    return ((Boolean) values.get("trackIntegrity")).booleanValue();
  }

  public Date getEndDate()
  {
    return (Date)values.get("endDate");
  }

  public Date getStartDate()
  {
    return (Date)values.get("startDate");
  }

  public String getOwner() {
    return (String)values.get("owner");
  }

  public boolean isDelivery() {
    return ((Boolean)values.get("delivery")).booleanValue();
  }
}
