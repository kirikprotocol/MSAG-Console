/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 5, 2003
 * Time: 4:12:54 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.mscman;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.mscman.MscDataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.mscman.MscQuery;

import java.util.List;

import org.apache.log4j.Category;

public class MscManager {
  private Category logger = Category.getInstance(MscManager.class);

  private Smsc smsc = null;
  private MscDataSource mscs = new MscDataSource();

  public void setSmsc(Smsc smsc)
  {
    this.smsc = smsc;
  }

  public Smsc getSmsc()
  {
    return smsc;
  }

  public QueryResultSet query(MscQuery query) throws AdminException
  {
    mscs.clear();
    List lst = smsc.mscList();
    for (int i = 0; i < lst.size(); i++) {
      Object obj = lst.get(i);
      if (obj != null && obj instanceof String) {
        try {
          mscs.add(new MscInfo((String) obj));
        } catch (IllegalArgumentException e) {
          logger.error("Misformatted msc string returned by smsc:\"" + (String) obj + "\", msc scipped", e);
        }
      }
    }
    return mscs.query(query);
  }

  public void register(String msc) throws AdminException
  {
    smsc.mscRegistrate(msc);
  }

  public void unregister(String msc) throws AdminException
  {
    smsc.mscUnregister(msc);
  }

  public void block(String msc) throws AdminException
  {
    smsc.mscBlock(msc);
  }

  public void clear(String msc) throws AdminException
  {
    smsc.mscClear(msc);
  }
}
