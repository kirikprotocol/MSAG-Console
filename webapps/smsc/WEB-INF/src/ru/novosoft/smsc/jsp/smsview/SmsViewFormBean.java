package ru.novosoft.smsc.jsp.smsview;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.smsview.SmsQuery;
import ru.novosoft.smsc.admin.smsview.SmsRow;
import ru.novosoft.smsc.admin.smsview.SmsSet;
import ru.novosoft.smsc.admin.smsview.SmsView;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import javax.servlet.ServletOutputStream;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

public class SmsViewFormBean extends IndexBean {

  private SmsQuery query = new SmsQuery();
  private SmsView view = new SmsView();

  private SmsSet rows = null;
  private SmsRow row = null;
  private String viewId = "";

  private int deletedRowsCount = 0;
  private int totalRowsCount = 0;

  public final static String UNKNOWN_STR = "unknown";
  public final static String ERR_CODES_PREFIX = "smsc.errcode.";
  public final static String ERR_CODE_UNKNOWN = ERR_CODES_PREFIX + UNKNOWN_STR;

  private String mbRemove = null;
  private String mbDelete = null;
  private String mbQuery = null;
  private String mbClear = null;
  private String mbView = null;
  private String mbSave = null;

  private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";
  private String oldSort = null;

  private Vector checkedRows = new Vector();
  private boolean exactRowsCount = false;

  public class ErrorValue implements Comparable {
    public int errorCode;
    public String errorString;

    public ErrorValue(int errorCode, String errorString) {
      this.errorCode = errorCode;
      this.errorString = errorString;
    }

    public int compareTo(Object o) {
      if (o == null || !(o instanceof ErrorValue)) return -1;
      return (this.errorCode - ((ErrorValue) o).errorCode);
    }
  }

  ;
  private Vector errorValues = null;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK) return result;

    if (sort != null)
      preferences.setSmsviewSortOrder(sort);
    else
      sort = preferences.getSmsviewSortOrder();

    try {
      view.init(appContext);
    } catch (Throwable t) {
      return error(SMSCErrors.error.smsview.InitFailed, t);
    }

    return RESULT_OK;
  }

  public int process(HttpServletRequest request, HttpServletResponse response) {
    int result = super.process(request);
    if (result != RESULT_OK) return result;

    if (errorValues == null) { // init error values
      Set errorStrings = appContext.getLocaleStrings(principal, ERR_CODES_PREFIX);
      errorValues = new Vector();
      for (Iterator i = errorStrings.iterator(); i.hasNext();) {
        String err = (String) i.next();
        if (!err.startsWith(UNKNOWN_STR)) {
          try {
            int errorCode = Integer.parseInt(err);
            String errorString = appContext.getLocaleString(principal, ERR_CODES_PREFIX + err);
            errorValues.add(new ErrorValue(errorCode, (errorString != null) ? errorString : ""));
          } catch (Exception e) {
          }
        }
      }
      Collections.sort(errorValues);
    }

    if (getStorageType() == SmsQuery.SMS_ARCHIVE_STORAGE_TYPE) {
      if (!request.isUserInRole("smsView") && !request.isUserInRole("smsView_archive"))
        return error(SMSCErrors.error.smsview.AccessDeniedToArchive);
    }
    if (getStorageType() == SmsQuery.SMS_OPERATIVE_STORAGE_TYPE) {
      if (!request.isUserInRole("smsView") && !request.isUserInRole("smsView_operative"))
        return error(SMSCErrors.error.smsview.AccessDeniedToOperative);
    }

    if (mbRemove != null)
      result = processDeleteSelected();
    else if (mbDelete != null) {
      if (rows != null) {
        result = processDeleteSet(rows);
      } else {
        result = processQuery();
      }
    } else if (mbQuery != null)
      result = processQuery();
    else if (mbClear != null)
      result = clearQuery();
    else if (mbView != null)
      result = viewQuery();
    else if (mbSave != null)
      try {
        response.setContentType("application/vnd.ms-excel");
        response.setHeader("Content-Disposition", "attachment; filename=smsview.xls");
        result = saveResultsToFile(response);
        journalAppend(SubjectTypes.TYPE_smsview, "Save results to xls-file", Actions.ACTION_SAVE);
      } catch (Throwable t) {
        mbSave = null;
        return error(SMSCErrors.error.smsview.CouldntSave, t);
      }
    else
      result = processResortAndNavigate(false);

    mbRemove = null;
    mbDelete = null;
    mbQuery = null;
    mbClear = null;
    mbView = null;
    mbSave = null;

    return result;
  }

  private int processResortAndNavigate(boolean force) {
    if (force || (sort != null && sort.length() > 0 && !sort.equals(oldSort))) {
      final boolean isNegativeSort = sort.startsWith("-");
      final String sortField = isNegativeSort ? sort.substring(1) : sort;

      if (rows != null && rows.getRowsList() != null) {
        Collections.sort(rows.getRowsList(), new Comparator() {
          public int compare(Object o1, Object o2) {
            int result = 0;
            SmsRow r1 = (SmsRow) o1;
            SmsRow r2 = (SmsRow) o2;
            if (sortField.equalsIgnoreCase("name")) {
              Long r1_v = new Long(r1.getId());
              Long r2_v = new Long(r2.getId());
              result = r1_v.compareTo(r2_v);
            }
            if (sortField.equalsIgnoreCase("sendDate"))
              result = r1.getSubmitTime().compareTo(r2.getSubmitTime());
            if (sortField.equalsIgnoreCase("lastDate")) {
              Date time1 = r1.getLastTryTime();
              Date time2 = r2.getLastTryTime();
              result = (time1 != null && time2 != null) ? time1.compareTo(time2) :
                      ((time1 == null && time2 != null) ? -1 : 1);
            }
            if (sortField.equalsIgnoreCase("from"))
              result = r1.getOriginatingAddress().compareTo(r2.getOriginatingAddress());
            if (sortField.equalsIgnoreCase("to"))
              result = r1.getDestinationAddress().compareTo(r2.getDestinationAddress());
            if (sortField.equalsIgnoreCase("status"))
              result = r1.getStatus().compareTo(r2.getStatus());

            return isNegativeSort ? -result : result;
          }
        });
      }
      oldSort = sort;
      startPosition = 0;
    }
    return RESULT_OK;
  }

  public int processQuery() {
    rows = null;
    startPosition = 0;
    totalSize = 0;
    totalRowsCount = 0;
    checkedRows.removeAllElements();
    try {
      if ((query.getStorageType() == SmsQuery.SMS_ARCHIVE_STORAGE_TYPE) &&
              (!hostsManager.getServiceInfo(Constants.ARCHIVE_DAEMON_SVC_ID).isOnline())) {
        clearQuery();
        throw new AdminException("Archive Daemon is not running. ");
      }

      if (!exactRowsCount) {
        rows = view.getSmsSet(query);
        if (query.getStorageType() == SmsQuery.SMS_OPERATIVE_STORAGE_TYPE)
          journalAppend(SubjectTypes.TYPE_smsview, "Query", Actions.ACTION_VIEW, "Operative storage", query.getJournalInfo());
        else if (query.getStorageType() == SmsQuery.SMS_ARCHIVE_STORAGE_TYPE)
          journalAppend(SubjectTypes.TYPE_smsview, "Query", Actions.ACTION_VIEW, "Archive storage", query.getJournalInfo());
        totalRowsCount = rows.getRowsCount();
      } else {
        int storage = query.getStorageType();
        if (storage == SmsQuery.SMS_OPERATIVE_STORAGE_TYPE) {
          rows = view.getOperativeCount(query);
          totalRowsCount = rows.getSmesRows();
          journalAppend(SubjectTypes.TYPE_smsview, "Query", Actions.ACTION_VIEW, "Operative storage", query.getJournalInfo());
        } else if (storage == SmsQuery.SMS_ARCHIVE_STORAGE_TYPE) {
          rows = view.getSmsSet(query);
          totalRowsCount = view.getArchiveCount(query);
          journalAppend(SubjectTypes.TYPE_smsview, "Query", Actions.ACTION_VIEW, "Archive storage", query.getJournalInfo());
        } else
          throw new AdminException("Unsupported storage type: " + storage);
        // totalRowsCount = view.getSmsCount(query);
        if (rows != null) rows.setHasMore(false);
      }
      startPosition = 0;
      totalSize = (rows == null) ? 0 : rows.getRowsCount();
      processResortAndNavigate(true);
      return RESULT_OK;
    } catch (AdminException ex) {
      ex.printStackTrace();
      return error(SMSCErrors.error.smsview.QueryFailed, ex.getMessage());
    }
  }

  public int clearQuery() {
    rows = null;
    startPosition = 0;
    totalSize = 0;
    totalRowsCount = 0;
    exactRowsCount = false;
    checkedRows.removeAllElements();
    query = new SmsQuery();
    processResortAndNavigate(true);
    return RESULT_OK;
  }

  public int viewQuery() {
    try {
      if (rows == null)
        throw new Exception("There are no messages selected");
      row = rows.getRow(viewId);
      if (row == null)
        throw new Exception("Message #" + viewId + " is not setected in main view");
      journalAppend(SubjectTypes.TYPE_smsview, "Detailed view:" + row.getIdString(), Actions.ACTION_VIEW);

      return RESULT_OK;
    } catch (Exception ex) {
      ex.printStackTrace();
      return error(SMSCErrors.error.smsview.QueryFailed, ex.getMessage());
    }
  }

  public int processDeleteSet(SmsSet set) {
    try {
      int storage = getStorageType();
      if (storage == SmsQuery.SMS_OPERATIVE_STORAGE_TYPE) {
        deletedRowsCount = view.delOperativeSmsSet(set);
        journalAppend(SubjectTypes.TYPE_smsview, deletedRowsCount + " deleted", Actions.ACTION_DEL);
      } else if (storage == SmsQuery.SMS_ARCHIVE_STORAGE_TYPE)
        throw new AdminException("Cancel is not suported for persistent archive storage!");
      else
        throw new AdminException("Storage type " + storage + " is invalid");
    } catch (AdminException ex) {
      ex.printStackTrace();
      return error(SMSCErrors.error.smsview.DeleteFailed, ex.getMessage());
    }

    return processQuery();
  }

  public int processDeleteSelected() {
    SmsSet set = new SmsSet();
    int rowsCount = (rows == null) ? 0 : rows.getRowsCount();
    for (int i = 0; i < rowsCount; i++) {
      SmsRow row = rows.getRow(i);
      String rowId = Long.toString(row.getId());
      if (checkedRows.contains(rowId)) set.addRow(row);
    }
    return processDeleteSet(set);
  }

  public SmsRow getRow(int index) {
    return rows == null ? null : rows.getRow(index);
  }

  public boolean isHasMore() {
    return rows != null && rows.isHasMore();
  }

  public void refreshQuery() {
    query = new SmsQuery();
    setFromDate(null);
    setTillDate(null);
    checkedRows.removeAllElements();
    exactRowsCount = false;
  }

  public int saveResultsToFile(HttpServletResponse response) throws Exception {
    if (rows.getRowsCount() == 0) return RESULT_ERROR;

    SimpleDateFormat dateFormatter = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss", appContext.getUserManager().getPrefs(principal).getLocale());
    StringBuffer result = new StringBuffer();
    for (int i = 0; i < rows.getRowsCount(); i++) {
      SmsRow row = rows.getRow(i);
      String id = Long.toString(row.getId());
      String submit = " ";
      if (row.getSubmitTime() != null)
        submit = StringEncoderDecoder.encode(dateFormatter.format(row.getSubmitTime()));
      String valid = " ";
      if (row.getValidTime() != null)
        valid = StringEncoderDecoder.encode(dateFormatter.format(row.getValidTime()));
      String last = " ";
      if (row.getLastTryTime() != null)
        last = StringEncoderDecoder.encode(dateFormatter.format(row.getLastTryTime()));
      String next = " ";
      if (row.getNextTryTime() != null)
        submit = StringEncoderDecoder.encode(dateFormatter.format(row.getNextTryTime()));
      String source = row.getOriginatingAddress();
      String dest = row.getDestinationAddress();
      String route = row.getRouteId();
      String status = row.getStatus();
      //id + "\t" + submit + "\t" + valid + "\t" + last + "\t" + next + "\t" + source + "\t" + dest + "\t" + route + "\t" + status + "\t";
      result.append(id).append('\t').append(submit).append('\t').append(valid).append('\t').append(last).append('\t').append(next).append('\t');
      result.append(source).append('\t').append(dest).append('\t').append(route).append('\t').append(status).append('\t');
      if (row.getText() != null) {
        row.getBodyParameters().get();
        result.append(row.getText());
      }
      result.append("\r\n");
    }
    byte[] buf = result.toString().getBytes();
    response.setContentLength(buf.length);
    ServletOutputStream out = response.getOutputStream();
    out.write(buf);
    out.close();

    return RESULT_OK;
  }

  /**
   * ****************************** query delegeates ********************************
   */

  public void setSort(String by) {
    sort = by;
  }

  public String getSort() {
    return sort;
  }

  public int getStorageType() {
    return query.getStorageType();
  }

  public void setStorageType(int type) {
    query.setStorageType(type);
  }

  public int getRowsMaximum() {
    return query.getRowsMaximum();
  }

  public void setRowsMaximum(int max) {
    query.setRowsMaximum(max);
  }

  public String getFromAddress() {
    return query.getFromAddress();
  }

  public void setAbonentAddress(String address) {
    query.setAbonentAddress(address);
  }

  public String getAbonentAddress() {
    return query.getAbonentAddress();
  }

  public void setFromAddress(String address) {
    query.setFromAddress(address);
  }

  public String getToAddress() {
    return query.getToAddress();
  }

  public void setToAddress(String address) {
    query.setToAddress(address);
  }

  public String getSmeId() {
    return query.getSmeId();
  }

  public void setSmeId(String id) {
    query.setSmeId(id);
  }

  public String getSrcSmeId() {
    return query.getSrcSmeId();
  }

  public void setSrcSmeId(String id) {
    query.setSrcSmeId(id);
  }

  public String getDstSmeId() {
    return query.getDstSmeId();
  }

  public void setDstSmeId(String id) {
    query.setDstSmeId(id);
  }

  public String getRouteId() {
    return query.getRouteId();
  }

  public void setRouteId(String id) {
    query.setRouteId(id);
  }

  public String getSmsId() {
    return query.getSmsId();
  }

  public void setSmsId(String id) {
    try {
      query.setSmsId(id);
    }
    catch (AdminException e) {
      error("Invalid numeric format for sms id", e);
    }
  }

  public int getStatus() {
    return query.getStatus();
  }

  public void setStatus(int status) {
    query.setStatus(status);
  }

  public int getLastResult() {
    return query.getLastResult();
  }

  public void setLastResult(int lastResult) {
    query.setLastResult(lastResult);
  }

  public String getFromDate() {
    if (query.getFilterFromDate()) {
      SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT, appContext.getUserManager().getPrefs(principal).getLocale());
      return formatter.format(query.getFromDate());
    } else
      return "";
  }

  public void setFromDate(String dateString) {
    final boolean dateEnabled = dateString != null && dateString.trim().length() > 0;
    query.setFilterFromDate(dateEnabled);
    if (dateEnabled) {
      try {
        SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT, appContext.getUserManager().getPrefs(principal).getLocale());
        query.setFromDate(formatter.parse(dateString));
      } catch (ParseException e) {
        query.setFromDate(new Date());
        e.printStackTrace();
      }
    }
  }

  public String getTillDate() {
    if (query.getFilterTillDate()) {
      SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT, appContext.getUserManager().getPrefs(principal).getLocale());
      return formatter.format(query.getTillDate());
    } else
      return "";
  }

  public void setTillDate(String dateString) {
    final boolean dateEnabled = dateString != null && dateString.trim().length() > 0;
    query.setFilterTillDate(dateEnabled);
    if (dateEnabled) {
      try {
        SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT, appContext.getUserManager().getPrefs(principal).getLocale());
        query.setTillDate(formatter.parse(dateString));
      } catch (ParseException e) {
        query.setTillDate(new Date());
        e.printStackTrace();
      }
    }
  }

  public int getTotalRowsCount() {
    return totalRowsCount;
  }

  public int getDeletedRowsCount() {
    return deletedRowsCount;
  }

  public String getMbDelete() {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete) {
    this.mbDelete = mbDelete;
  }

  public String getMbRemove() {
    return mbRemove;
  }

  public void setMbRemove(String mbRemove) {
    this.mbRemove = mbRemove;
  }

  public String getMbQuery() {
    return mbQuery;
  }

  public void setMbQuery(String mbQuery) {
    this.mbQuery = mbQuery;
  }

  public String getMbClear() {
    return mbClear;
  }

  public void setMbClear(String mbClear) {
    this.mbClear = mbClear;
  }

  public String getMbView() {
    return mbView;
  }

  public void setMbView(String mbView) {
    this.mbView = mbView;
  }

  public String[] getCheckedRows() {
    return (checkedRows == null) ? null : (String[]) checkedRows.toArray();
  }

  public void setCheckedRows(String[] checkedRows) {
    for (int i = 0; i < checkedRows.length; i++)
      this.checkedRows.addElement(checkedRows[i]);
  }

  public boolean isRowChecked(long id) {
    return (checkedRows != null) && checkedRows.contains(Long.toString(id));
  }

  public String getViewId() {
    return viewId;
  }

  public void setViewId(String viewId) {
    this.viewId = viewId;
  }

  public SmsRow getRow() {
    return row;
  }

  public boolean isExactRowsCount() {
    return exactRowsCount;
  }

  public void setExactRowsCount(boolean exactRowsCount) {
    this.exactRowsCount = exactRowsCount;
  }

  public Collection getErrorValues() {
    return errorValues;
  }

  public String getMbSave() {
    return mbSave;
  }

  public void setMbSave(String mbSave) {
    this.mbSave = mbSave;
  }
}
