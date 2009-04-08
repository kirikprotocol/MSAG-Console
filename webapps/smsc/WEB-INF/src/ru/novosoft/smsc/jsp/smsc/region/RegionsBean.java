package ru.novosoft.smsc.jsp.smsc.region;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.region.RegionsManager;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCJspException;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.regions.RegionsDataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.regions.RegionsFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.regions.RegionsQuery;

import javax.servlet.http.HttpServletRequest;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * User: artem
 * Date: Aug 1, 2007
 */

public class RegionsBean extends SmscBean {

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

  private String defaultBandwidth;
  private String defaultEmail;
  private QueryResultSet rows = new EmptyResultSet();
  private String sort = "name";
  private int startPosition;
  private int pageSize = 20;
  private int totalSize;
  private String[] checked = new String[0];
  private Set checkedSet = new HashSet();
  private RegionsManager rm;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != PageBean.RESULT_OK)
      return result;

    rm = appContext.getRegionsManager();

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

    final RegionsDataSource ds;
    try {
      ds = new RegionsDataSource(appContext);
      rows = ds.query(new RegionsQuery(pageSize, sort, startPosition, new RegionsFilter(null)));
      totalSize = rows.getTotalSize();
    } catch (AdminException e) {
      logger.error(e,e);
      return _error(new SMSCJspException("Can't load regions", SMSCJspException.ERROR_CLASS_ERROR, e));
    }
    if (defaultBandwidth == null)
      defaultBandwidth = String.valueOf(rm.getDefaultBandwidth());
    if (defaultEmail == null)
      defaultEmail = rm.getDefaultEmail();

    checkedSet.addAll(Arrays.asList(checked));

    if (rm.isModified())
      warning("Regions configuration changed.");

    return PageBean.RESULT_OK;
  }

  private int reset() {
    try {
      rm.reset();
      defaultBandwidth = null;
      defaultEmail = null;
      journalAppend(SubjectTypes.TYPE_regions, "Regions configuration", Actions.ACTION_RESTORE);
    } catch (AdminException e) {
      logger.error(e,e);
      return error("Can't reset regions", e);
    }
    return PageBean.RESULT_OK;
  }

  private int save() {
    try {
      if (defaultBandwidth == null || defaultBandwidth.length() == 0)
        return error("Default bandwidth is empty");

      try {
        rm.setDefaultBandwidth(Integer.parseInt(defaultBandwidth));
      } catch (NumberFormatException e) {
        return error("Invalid default bandwidth");
      }

      rm.setDefaultEmail(defaultEmail);

      rm.save();

      journalAppend(SubjectTypes.TYPE_regions, "Regions configuration", Actions.ACTION_MODIFY);
    } catch (AdminException e) {
      logger.error(e,e);
      return error("Can't save regions", e);
    }
    return PageBean.RESULT_OK;
  }

  private int delete() {
    try {
      for (int i = 0; i < checked.length; i++)
        rm.removeRegion(Integer.parseInt(checked[i]));
    } catch (AdminException e) {
      logger.error(e,e);
      return error("Can't delete regions", e);
    }
    return PageBean.RESULT_OK;
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

  public String getDefaultBandwidth() {
    return defaultBandwidth;
  }

  public void setDefaultBandwidth(String defaultBandwidth) {
    this.defaultBandwidth = defaultBandwidth;
  }

  public String getDefaultEmail() {
    return defaultEmail;
  }

  public void setDefaultEmail(String defaultEmail) {
    this.defaultEmail = defaultEmail;
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

  public boolean isRegionChecked(String id) {
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
