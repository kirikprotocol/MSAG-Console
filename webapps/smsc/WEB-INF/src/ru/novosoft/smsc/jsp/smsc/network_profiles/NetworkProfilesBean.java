package ru.novosoft.smsc.jsp.smsc.network_profiles;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.network_profiles.NetworkProfilesManager;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCJspException;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.network_profiles.NetworkProfileQuery;
import ru.novosoft.smsc.jsp.util.tables.impl.network_profiles.NetworkProfilesDataSource;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class NetworkProfilesBean extends SmscBean {

  public static final int RESULT_REFRESH = SmscBean.PRIVATE_RESULT + 1;
  public static final int RESULT_ADD = SmscBean.PRIVATE_RESULT + 2;
  public static final int RESULT_DELETE = SmscBean.PRIVATE_RESULT + 3;
  public static final int RESULT_EDIT = SmscBean.PRIVATE_RESULT + 4;

  private String mbAdd = null;
  private String mbEdit = null;
  private String mbDelete = null;
  private String mbSave = null;
  private String mbReset = null;
  private String edit;

  private String defaultAbonentStatusMethod;
  private String defaultVlrDetectionMethod;
  private String defaultUssdOpenDestRef;
  private QueryResultSet rows = new EmptyResultSet();
  private String sort = "name";
  private int startPosition;
  private int pageSize = 20;
  private int totalSize;
  private String[] checked = new String[0];
  private Set checkedSet = new HashSet();
  private NetworkProfilesManager nm;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != PageBean.RESULT_OK)
      return result;

    nm = appContext.getNetworkProfilesManager();

    return PageBean.RESULT_OK;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != PageBean.RESULT_OK) return result;

    if (mbAdd != null) {
      mbAdd = null;
      return RESULT_ADD;
    } else if (mbEdit != null) {
      mbEdit = null;
      return RESULT_EDIT;
    } else if (mbDelete != null) {
      mbDelete = null;
      delete();
    } else if (mbSave != null) {
      mbSave = null;
      save();
    } else if (mbReset != null) {
      mbReset = null;
      reset();
    }

    final NetworkProfilesDataSource ds;
    try {
      ds = new NetworkProfilesDataSource(appContext);
      rows = ds.query(new NetworkProfileQuery(pageSize, sort, startPosition));
      totalSize = rows.getTotalSize();
    } catch (AdminException e) {
      logger.error(e,e);
      return _error(new SMSCJspException("Can't load profiles", SMSCJspException.ERROR_CLASS_ERROR, e));
    }
    if (defaultAbonentStatusMethod == null)
      defaultAbonentStatusMethod = nm.getDefaultAbonentStatusMethod();
    if (defaultVlrDetectionMethod == null)
      defaultVlrDetectionMethod = nm.getDefaultVlrDetectionMethod();
    if (defaultUssdOpenDestRef == null)
      defaultUssdOpenDestRef = nm.getDefaultUssdOpenDestRef();

    checkedSet.addAll(Arrays.asList(checked));

    if (nm.isModified())
      warning("Profiles configuration changed.");

    return PageBean.RESULT_OK;
  }

  private int reset() {
    try {
      nm.reset();
      defaultAbonentStatusMethod = null;
      defaultVlrDetectionMethod = null;
      defaultUssdOpenDestRef = null;
      journalAppend(SubjectTypes.TYPE_network_profiles, "Network profiles configuration", Actions.ACTION_RESTORE);
    } catch (AdminException e) {
      logger.error(e,e);
      return error("Can't reset profiles", e);
    }
    return PageBean.RESULT_OK;
  }

  private int save() {

    try {

      nm.setDefaultAbonentStatusMethod(defaultAbonentStatusMethod);

      nm.setDefaultVlrDetectionMethod(defaultVlrDetectionMethod);

      nm.setDefaultUssdOpenDestRef(defaultUssdOpenDestRef);

      nm.save();

      appContext.getSmsc().applyNetworkProfilesConfig();

      journalAppend(SubjectTypes.TYPE_network_profiles, "Network profiles configuration", Actions.ACTION_MODIFY);
    } catch (AdminException e) {
      logger.error(e,e);
      return error("Can't save profiles", e);
    }
    return PageBean.RESULT_OK;
  }

  private int delete() {
    try {
      for (int i = 0; i < checked.length; i++)
        nm.removeProfile(checked[i]);
    } catch (AdminException e) {
      logger.error(e,e);
      return error("Can't delete profiles", e);
    }
    return PageBean.RESULT_OK;
  }


  public List getAbonentStateMethods() {
    List list = new ArrayList(2);
    list.add("SRI4SM");
    list.add("ATI");
    return list;
  }

  public List getVlrDetectionMethods() {
    List list = new ArrayList(2);
    list.add("SRI4SM");
    list.add("ATI");
    return list;
  }

  public List getUssdOpenDestRefs() {
    List list = new ArrayList(3);
    list.add("");
    list.add("destAddr");
    list.add("destIMSI");
    return list;
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

  public String getMbEdit() {
    return mbEdit;
  }

  public void setMbEdit(String mbEdit) {
    this.mbEdit = mbEdit;
  }

  public String getDefaultAbonentStatusMethod() {
    return defaultAbonentStatusMethod;
  }

  public void setDefaultAbonentStatusMethod(String defaultAbonentStatusMethod) {
    this.defaultAbonentStatusMethod = defaultAbonentStatusMethod;
  }

  public String getDefaultVlrDetectionMethod() {
    return defaultVlrDetectionMethod;
  }

  public void setDefaultVlrDetectionMethod(String defaultVlrDetectionMethod) {
    this.defaultVlrDetectionMethod = defaultVlrDetectionMethod;
  }

  public String getDefaultUssdOpenDestRef() {
    return defaultUssdOpenDestRef;
  }

  public void setDefaultUssdOpenDestRef(String defaultUssdOpenDestRef) {
    this.defaultUssdOpenDestRef = defaultUssdOpenDestRef;
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

  public QueryResultSet getRows() {
    return rows;
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

  public boolean isProfileChecked(String id) {
    return checkedSet.contains(id);
  }

  public String[] getChecked() {
    return checked;
  }

  public void setChecked(String[] checked) {
    this.checked = checked;
  }

  public String getEdit() {
    return edit;
  }

  public void setEdit(String edit) {
    this.edit = edit;
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
}
