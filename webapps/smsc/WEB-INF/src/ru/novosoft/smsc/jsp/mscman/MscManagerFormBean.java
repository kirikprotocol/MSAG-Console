package ru.novosoft.smsc.jsp.mscman;

/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 5, 2003
 * Time: 4:04:15 PM
 * To change this template use Options | File Templates.
 */

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.mscman.MscManager;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.mscman.MscFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.mscman.MscQuery;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

public class MscManagerFormBean extends IndexBean
{
  private MscManager manager = new MscManager();
  private QueryResultSet mscs = new EmptyResultSet();

  private String mbRegister = null;
  private String mbUnregister = null;
  private String mbBlock = null;
  private String mbClear = null;

  private String mscNum = "";
  private String mscKey = "";

  private String prefix = "";
  private boolean refreshed = false;

  protected int init(List errors)
  {
    pageSize = 10;
    if (sort == null)
      sort = "commutator";

    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    return RESULT_OK;
  }

  private void clearBeenProperties()
  {
    mbRegister = null;
    mbUnregister = null;
    mbBlock = null;
    mbClear = null;
    mscNum = "";
    mscKey = "";
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK) {
      clearBeenProperties();
      return result;
    }

    if (manager.getSmsc() == null) {
      manager.setSmsc(appContext.getSmsc());
    }


    try {
      if (mbBlock != null) {
        manager.block(mscKey);
        journalAppend(SubjectTypes.TYPE_msc, mscKey, Actions.ACTION_LOCK);
      }
      else if (mbClear != null) {
        manager.clear(mscKey);
        journalAppend(SubjectTypes.TYPE_msc, mscKey, Actions.ACTION_UNLOCK);
      }
      else if (mbUnregister != null) {
        manager.unregister(mscKey);
        journalAppend(SubjectTypes.TYPE_msc, mscKey, Actions.ACTION_DEL);
      }
      else if (mbRegister != null) {
        manager.register(mscNum);
        journalAppend(SubjectTypes.TYPE_msc, mscKey, Actions.ACTION_ADD);
      }

      mscs = manager.query(new MscQuery(pageSize, new MscFilter(prefix), sort, startPosition));
      totalSize = mscs.getTotalSize();
      result = refreshed ? RESULT_OK : RESULT_DONE;
      refreshed = false;
    } catch (AdminException exc) {
      exc.printStackTrace();
      result = error(SMSCErrors.error.commutators.smscServerError, exc);
    }
    clearBeenProperties();
    return result;
  }

  public QueryResultSet getMscs()
  {
    return mscs;
  }

  public String getMscNum()
  {
    return mscNum;
  }

  public void setMscNum(String mscNum)
  {
    this.mscNum = mscNum;
  }

  public String getMscKey()
  {
    return mscKey;
  }

  public void setMscKey(String mscKey)
  {
    this.mscKey = mscKey;
  }

  public void setMbRegister(String mbRegister)
  {
    this.mbRegister = mbRegister;
  }

  public void setMbUnregister(String mbUnregister)
  {
    this.mbUnregister = mbUnregister;
  }

  public void setMbBlock(String mbBlock)
  {
    this.mbBlock = mbBlock;
  }

  public void setMbClear(String mbClear)
  {
    this.mbClear = mbClear;
  }

  public String getPrefix()
  {
    return prefix;
  }

  public void setPrefix(String prefix)
  {
    this.prefix = prefix == null ? "" : prefix;
  }

  public boolean isRefreshed()
  {
    return refreshed;
  }

  public void setRefreshed(boolean refreshed)
  {
    this.refreshed = refreshed;
  }
}
