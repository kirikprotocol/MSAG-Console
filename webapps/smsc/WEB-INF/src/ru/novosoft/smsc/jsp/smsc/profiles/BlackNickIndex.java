package ru.novosoft.smsc.jsp.smsc.profiles;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.profiler.BlackNick;
import ru.novosoft.smsc.jsp.SMSCJspException;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.blacknick.BlackNickQuery;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * User: artem
 * Date: 18.07.2007
 */

public class BlackNickIndex extends IndexBean {
  public static final int RESULT_ADD = IndexBean.PRIVATE_RESULT;
  public static final int RESULT_REMOVE = IndexBean.PRIVATE_RESULT + 1;
  public static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 2;

  private QueryResultSet rs;

  private String filter="";
  private String newNick;
  private String[] checked = new String[0];
  private Set checkedSet = new HashSet();

  private String mbAdd = null;
  private String mbDelete = null;
  private String mbQuery = null;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    pageSize = preferences.getBlackNickPageSize();
    if (sort == null)
      sort = "nick";
    return RESULT_OK;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    rs = new EmptyResultSet();

    if (mbAdd != null) {
      mbAdd = null;
      result = add();
    } else if (mbDelete != null) {
      mbDelete = null;
      result = delete();
    }
    if (result != RESULT_OK)
      return result;

    checkedSet.addAll(Arrays.asList(checked));

    return query();
  }

  private int add() {
    try {
      smsc.addBlackNick(new BlackNick(newNick.toLowerCase()));
    } catch (AdminException e) {
      _error(new SMSCJspException("Can't add black nick", SMSCJspException.ERROR_CLASS_WARNING, e));
    }
    return RESULT_OK;
  }

  private int delete() {
    ArrayList blackNicks = new ArrayList();
    for (int i = 0; i < checked.length; i++)
      blackNicks.add(new BlackNick(checked[i]));

    try {
      smsc.removeBlackNicks(blackNicks);
    } catch (AdminException e) {
       _error(new SMSCJspException("Can't remove black nicks", SMSCJspException.ERROR_CLASS_WARNING, e));
    }
    return RESULT_OK;
  }

  private int query() {
    try {
      rs = smsc.blackNicksQueryFromFile(new BlackNickQuery(preferences.getBlackNickMaxSize(), filter, sort));
      totalSize = rs.getTotalSize();
      if (startPosition >= totalSize)
        startPosition = Math.round(totalSize/pageSize) * pageSize;

      return RESULT_OK;
    } catch (AdminException e) {
      return _error(new SMSCJspException("Can't query black nick list", SMSCJspException.ERROR_CLASS_ERROR, e));
    }
  }

  public String[] getChecked() {
    return checked;
  }

  public void setChecked(String[] checked) {
    this.checked = checked;
  }

  public String getNewNick() {
    return newNick;
  }

  public void setNewNick(String newNick) {
    this.newNick = newNick;
  }

  public String getMbAdd() {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd) {
    this.mbAdd = mbAdd;
  }

  public String getMbDelete() {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete) {
    this.mbDelete = mbDelete;
  }

  public String getMbQuery() {
    return mbQuery;
  }

  public void setMbQuery(String mbQuery) {
    this.mbQuery = mbQuery;
  }

  public String getFilter() {
    return filter;
  }

  public void setFilter(String filter) {
    this.filter = filter;
  }

  public boolean isNickChecked(String nick) {
    return checkedSet.contains(nick);
  }

  public QueryResultSet getRs() {
    return rs;
  }
}
