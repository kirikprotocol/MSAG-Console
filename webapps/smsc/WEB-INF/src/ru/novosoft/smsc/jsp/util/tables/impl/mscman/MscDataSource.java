package ru.novosoft.smsc.jsp.util.tables.impl.mscman;

import ru.novosoft.smsc.admin.mscman.MscInfo;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;

/**
 * Created by igork
 * Date: Aug 18, 2003
 * Time: 3:06:54 PM
 */
public class MscDataSource extends AbstractDataSourceImpl {
  public MscDataSource()
  {
    super(new String[]{"commutator", "status", "failures"});
  }

  public void add(MscInfo info)
  {
    super.add(new MscDataItem(info));
  }

  public void remove(MscInfo info)
  {
    super.remove(new MscDataItem(info));
  }
}
