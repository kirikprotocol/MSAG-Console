/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 25, 2003
 * Time: 12:02:59 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.dl;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

public class Principal extends AbstractDataItem
{
  public Principal(String address, int maxLists, int maxElements)
  {
    values.put("address", address);
    values.put("max_lst", new Integer(maxLists));
    values.put("max_el", new Integer(maxElements));
  }

  public String getAddress()
  {
    return (String) values.get("address");
  }

  public void setAddress(String address)
  {
    values.put("address", address);
  }

  public int getMaxLists()
  {
    return ((Integer)values.get("max_lst")).intValue();
  }

  public void setMaxLists(int maxLists)
  {
    values.put("max_lst", new Integer(maxLists));
  }

  public int getMaxElements()
  {
    return ((Integer)values.get("max_el")).intValue();
  }

  public void setMaxElements(int maxElements)
  {
    values.put("max_el", new Integer(maxElements));
  }
}
