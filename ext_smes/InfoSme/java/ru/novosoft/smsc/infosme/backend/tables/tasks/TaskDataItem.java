package ru.novosoft.smsc.infosme.backend.tables.tasks;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

/**
 * Created by igork
 * Date: Sep 2, 2003
 * Time: 12:58:13 PM
 */
public class TaskDataItem extends AbstractDataItem {
  public TaskDataItem(String id, String name, String provider, boolean enabled, int priority, boolean retryOnFail, boolean replaceMessage, String svcType)
  {
    values.put("id", id);
    values.put("name", name);
    values.put("provider", provider);
    values.put("enabled", new Boolean(enabled));
    values.put("priority", new Integer(priority));
    values.put("retryOnFail", new Boolean(retryOnFail));
    values.put("replaceMessage", new Boolean(replaceMessage));
    values.put("svcType", svcType);
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
}
