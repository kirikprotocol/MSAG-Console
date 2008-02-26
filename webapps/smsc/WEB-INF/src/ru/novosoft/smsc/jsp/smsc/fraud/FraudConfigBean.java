package ru.novosoft.smsc.jsp.smsc.fraud;

import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCJspException;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.admin.fraud.FraudConfigManager;
import ru.novosoft.smsc.admin.AdminException;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * User: artem
 * Date: 26.02.2008
 */

public class FraudConfigBean extends SmscBean {


  private String mbAdd;
  private String mbDelete;
  private String mbSave;
  private String mbReset;
  private String tail;
  private Boolean enableCheck;
  private Boolean enableReject;
  private String newMsc="";

  private int startPosition;
  private int pageSize = 20;
  private int totalSize;
  private String[] checked = new String[0];
  private Set checkedSet = new HashSet();
  private String sort = "msc";

  private FraudConfigManager fcm;
  private List rows = new LinkedList();


  protected int init(List errors) {
    int result = super.init(errors);
    if (result != PageBean.RESULT_OK)
      return result;

    fcm = appContext.getFraudConfigManager();

    return PageBean.RESULT_OK;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != PageBean.RESULT_OK) return result;

    if (mbAdd != null) {
      result = add();
    } else if (mbDelete != null) {
      result = delete();
    } else if (mbSave != null) {
      result = save();
    } else if (mbReset != null) {
      result = reset();
    }

    query();
    if (tail == null)
      tail = String.valueOf(fcm.getTail());
    if (enableCheck == null)
      enableCheck = Boolean.valueOf(fcm.isEnableCheck());
    if (enableReject == null)
      enableReject = Boolean.valueOf(fcm.isEnableReject());

    return RESULT_OK;
  }

  private void query() {
    rows.clear();

    final Set newrows = new TreeSet(new Comparator() {
      public int compare(Object o1, Object o2) {
        String msc1 = (String)o1;
        String msc2 = (String)o2;
        return (sort.equals("msc")) ? msc1.compareTo(msc2) : -msc1.compareTo(msc2);
      }
    });

    newrows.addAll(fcm.getWhiteMscAddresses());
    totalSize = newrows.size();

    int i=0;
    for (Iterator iter = newrows.iterator(); iter.hasNext();i++) {
      String mscAddress = (String)iter.next();
      if (i >= startPosition && i < startPosition + pageSize)
        rows.add(mscAddress);
    }
  }

  private int add() {
    if (newMsc == null || newMsc.trim().length() == 0)
      return _error(new SMSCJspException("Msc address is empty", SMSCJspException.ERROR_CLASS_ERROR));

    if (!newMsc.matches("\\d+") || newMsc.length() > 15)
      return error("Invalid msc address");



    try {
      appContext.getFraudConfigManager().addWhiteMscAddress(newMsc);
    } catch (AdminException e) {
      logger.error(e,e);
      return error("Can't add msc", e);
    }

    newMsc = "";

    return RESULT_OK;
  }

  private int delete() {
    try {
      for (int i = 0; i < checked.length; i++)
        fcm.removeWhiteMscAddress(checked[i]);
    } catch (AdminException e) {
      logger.error(e,e);
      return error("Can't delete mscs", e);
    }
    return PageBean.RESULT_OK;
  }

  private int save() {
    if (tail == null || tail.trim().length() == 0)
      return error("Invalid tail");

    int tailInt;
    try {
      tailInt = Integer.parseInt(tail);
    } catch (NumberFormatException e) {
      return error("Invalid tail");
    }

    fcm.setTail(tailInt);

    fcm.setEnableCheck(enableCheck != null && enableCheck.booleanValue());
    fcm.setEnableReject(enableReject!= null && enableReject.booleanValue());
    try {
      fcm.save();

      smsc.applyFraud();
    } catch (AdminException e) {
      logger.error(e,e);
      return error("Can't save fraud config", e);
    }


    return RESULT_OK;
  }

  private int reset() {
    try {
      fcm.reset();
      tail = String.valueOf(fcm.getTail());
      enableCheck = Boolean.valueOf(fcm.isEnableCheck());
      enableReject = Boolean.valueOf(fcm.isEnableReject());
    } catch (AdminException e) {
      logger.error(e,e);
      return error("Can't reset", e);
    }
    return RESULT_OK;
  }

  public String getSort() {
    return sort;
  }

  public void setSort(String sort) {
    this.sort = sort;
  }

  public String getStartPosition() {
    return String.valueOf(startPosition);
  }

  public int getStartPositionInt() {
    return startPosition;
  }

  public void setStartPosition(int startPosition) {
    this.startPosition = startPosition;
  }

  public void setStartPosition(String startPosition) {
    try {
      this.startPosition = Integer.parseInt(startPosition);
    } catch (NumberFormatException e) {
      logger.error(e,e);
      this.startPosition = 0;
    }
  }

  public List getRows() {
    return new LinkedList(rows);
  }

  public String getPageSize() {
    return String.valueOf(pageSize);
  }

  public int getPageSizeInt() {
    return pageSize;
  }

  public int getTotalSizeInt() {
    return totalSize;
  }

  public boolean isMscChecked(String id) {
    return checkedSet.contains(id);
  }

  public String[] getChecked() {
    return checked;
  }

  public void setChecked(String[] checked) {
    this.checked = checked;
  }

  public String getMbSave() {
    return mbSave;
  }

  public void setMbSave(String mbSave) {
    this.mbSave = mbSave;
  }

  public String getMbReset() {
    return mbReset;
  }

  public void setMbReset(String mbReset) {
    this.mbReset = mbReset;
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

  public String getTail() {
    return tail;
  }

  public void setTail(String tail) {
    this.tail = tail;
  }

  public String getNewMsc() {
    return newMsc;
  }

  public void setNewMsc(String newMsc) {
    this.newMsc = newMsc;
  }


  public boolean isEnableCheck() {
    return enableCheck.booleanValue();
  }

  public boolean isEnableReject() {
    return enableReject.booleanValue();
  }

  public void setEnableCheck(boolean enableCheck) {
    this.enableCheck = Boolean.valueOf(enableCheck);
  }

  public void setEnableReject(boolean enableReject) {
    this.enableReject = Boolean.valueOf(enableReject);
  }
}
