package ru.novosoft.smsc.emailsme.backend;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 15.09.2003
 * Time: 16:52:16
 */
public class ProfilesDataItem extends AbstractDataItem
{
  public ProfilesDataItem(String addr, String userid, int dayLimit, String forward)
  {
    values.put("addr", addr);
    values.put("userid", userid);
    values.put("dayLimit", new Integer(dayLimit));
    values.put("forward", forward);
  }

  public String getAddr()
  {
    return (String) values.get("addr");
  }

  public String getUserid()
  {
    return (String) values.get("userid");
  }

  public int getDayLimit()
  {
    return ((Integer) values.get("dayLimit")).intValue();
  }

  public String getForward()
  {
    return (String) values.get("forward");
  }
}
