package ru.novosoft.smsc.jsp.util.tables.impl.provider;

/**
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

public class ProviderDataItem extends AbstractDataItem
{
  public ProviderDataItem(Provider provider)
  {
    values.put("id", String.valueOf(provider.getId()));
    values.put("name", provider.getName());
  }

  public String getId()
  {
    return (String) values.get("id");
  }

  public String getName()
  {
    return (String) values.get("name");
  }

}
