package ru.novosoft.smsc.jsp.util.tables.impl.mscman;

import ru.novosoft.smsc.admin.mscman.MscInfo;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

/**
 * Created by igork
 * Date: Aug 18, 2003
 * Time: 3:06:40 PM
 */
public class MscDataItem extends AbstractDataItem
{
  private boolean mLock = false;
  private boolean aLock = false;

  public MscDataItem(MscInfo info)
  {
    values.put("commutator", info.getMscNum());
    values.put("status", info.getLockString());
    values.put("failures", new Integer(info.getfCount()));
    this.mLock = info.ismLock();
    this.aLock = info.isaLock();
  }

  public String getCommutator()
  {
    return (String) values.get("commutator");
  }

  public String getStatus()
  {
    return (String) values.get("status");
  }

  public int getFailuresCount()
  {
    return ((Integer) values.get("failures")).intValue();
  }

  public boolean ismLock()
  {
    return mLock;
  }

  public boolean isaLock()
  {
    return aLock;
  }
}
