package ru.novosoft.smsc.emailsme.backend;

import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 15.09.2003
 * Time: 18:22:45
 * To change this template use Options | File Templates.
 */
public class ProfilesFilter implements Filter
{
  private String addr = null;
  private String userid = null;
  private int dayLimit = -1;
  private String forward = null;

  public ProfilesFilter(String addr, String userid, int dayLimit, String forward)
  {
    this.addr = addr;
    this.userid = userid;
    this.dayLimit = dayLimit;
    this.forward = forward;
  }

  public boolean isEmpty()
  {
    return addr == null && userid == null && dayLimit == -1 && forward == null;
  }

  public boolean isItemAllowed(DataItem item)
  {
    ProfilesDataItem pitem = (ProfilesDataItem) item;
    return (addr == null || pitem.getAddr().startsWith(addr))
           && (userid == null || pitem.getUserid().startsWith(userid))
           && (dayLimit == -1 || pitem.getDayLimit() == dayLimit)
           && (forward == null || pitem.getForward().startsWith(forward));
  }

  public String getAddr()
  {
    return addr;
  }

  public String getUserid()
  {
    return userid;
  }

  public int getDayLimit()
  {
    return dayLimit;
  }

  public String getForward()
  {
    return forward;
  }
}
