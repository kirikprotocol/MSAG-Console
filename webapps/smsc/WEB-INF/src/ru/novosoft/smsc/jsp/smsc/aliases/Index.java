package ru.novosoft.smsc.jsp.smsc.aliases;


import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.SMSCJspException;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasDataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasQuery;
import ru.novosoft.smsc.jsp.util.tables.table.Column;
import ru.novosoft.smsc.jsp.util.tables.table.DataCell;
import ru.novosoft.smsc.jsp.util.tables.table.PagedTableBean;
import ru.novosoft.smsc.jsp.util.tables.table.Row;
import ru.novosoft.smsc.jsp.util.tables.table.cell.CheckBoxCell;
import ru.novosoft.smsc.jsp.util.tables.table.cell.StringCell;
import ru.novosoft.smsc.jsp.util.tables.table.column.sortable.TextColumn;
import ru.novosoft.smsc.util.Functions;

import javax.servlet.http.HttpServletRequest;
import java.util.Iterator;
import java.util.List;

/**
 * User: artem
 * Date: 21.12.2006
 */
public class Index extends PagedTableBean {

  public static final int RESULT_ADD = PRIVATE_RESULT;
  public static final int RESULT_EDIT = PRIVATE_RESULT + 1;

  private final TextColumn checkColumn = new TextColumn(this, "", false);
  private final TextColumn aliasColumn = new TextColumn(this, "common.sortmodes.alias", false, 60);
  private final TextColumn addressColumn = new TextColumn(this, "common.sortmodes.address", false, 20);
  private final TextColumn hideColumn = new TextColumn(this, "common.sortmodes.hide", false, 20);

  protected String[] aliases = null;
  protected String[] addresses = null;
  protected byte hide = AliasFilter.HIDE_UNKNOWN;

  protected String mbQuery = null;

  protected String mbAdd = null;
  protected String mbDelete = null;
  protected String mbClear = null;

  private DataItem selectedAlias = null;

  private boolean initialized = false;

  private QueryResultSet aliasesList = null;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    setPageSize(preferences.getAliasesPageSize());

    try {
      aliases = (aliases == null) ? new String[0] : MaskList.normalizeMaskList(Functions.trimStrings(aliases));
      addresses = (addresses == null) ? new String[0] : MaskList.normalizeMaskList(Functions.trimStrings(addresses));

    } catch (AdminException e) {
      logger.error(e);
      return _error(new SMSCJspException("Init error", SMSCJspException.ERROR_CLASS_ERROR, e));
    }
    return RESULT_OK;
  }

  protected int doProcess(HttpServletRequest request) {
    if (hasTemporaryContent())
      aliasesList = (QueryResultSet)getFromTemporaryContent("aliasesList");

    initialized = aliasesList != null && aliasesList.size() > 0;
    System.out.println(initialized);

    int result = RESULT_OK;
    if (mbAdd != null) {
      mbAdd = null;
      result = processAdd();

    } else if (mbDelete != null) {
      mbDelete = null;
      result = processDelete();

    } else if (mbQuery != null) {
      mbQuery = null;
      result = processQuery();

    } else if (mbClear != null) {
      mbClear = null;
      result = processClear();
    }

    addToTemporaryContent("aliasesList", aliasesList);

    return result;
  }

  private int processAdd() {
    aliasesList = null;
    return RESULT_ADD;
  }

  private int processDelete() {
    for (Iterator iter = getCellsByColumn(checkColumn).iterator(); iter.hasNext();) {
      final CheckBoxCell cell = (CheckBoxCell)iter.next();
      if (cell.isChecked()) {
        appContext.getSmsc().getAliases().remove((String)cell.getObject());
        journalAppend(SubjectTypes.TYPE_alias, (String)cell.getObject(), Actions.ACTION_DEL);
        appContext.getStatuses().setAliasesChanged(true);
      }
    }
    return processQuery();
  }

  private int processClear() {
    aliasesList = null;
    initialized = false;
    return RESULT_OK;
  }

  private int processQuery() {
    aliasesList = null;
    initialized = true;

    final AliasFilter filter = new AliasFilter();
    try {
      filter.setAddresses(addresses);
    } catch (AdminException e) {
      return error(SMSCErrors.error.aliases.invalidAddress, e);
    }

    try {
      filter.setAliases(aliases);
    } catch (AdminException e) {
      return error(SMSCErrors.error.aliases.invalidAlias, e);
    }

    filter.setHide(hide);

    try {
      aliasesList = appContext.getSmsc().getAliases().query(createQuery(filter));
    } catch (AdminException e) {
      return _error(new SMSCJspException("Can't get aliases", SMSCJspException.ERROR_CLASS_ERROR, e));
    }

    if (aliasesList.size() > preferences.getMaxAliasesTotalSize())
      _error(new SMSCJspException("Query results is very big. Show first " + String.valueOf(preferences.getMaxAliasesTotalSize()+1) + " records.", SMSCJspException.ERROR_CLASS_WARNING));

    return RESULT_OK;
  }

  private AliasQuery createQuery(AliasFilter filter) {
    return new AliasQuery(preferences.getMaxAliasesTotalSize()+1, filter, AliasDataSource.ALIAS_FIELD, 0);
  }

  protected void fillTable(int start, int size) throws AdminException {

    if (aliasesList != null) {

      for (int i=start; i< aliasesList.size() && i < start + size; i++) {
        final DataItem item = (DataItem)aliasesList.get(i);

        final Row row = createNewRow();

        row.addCell(checkColumn, new CheckBoxCell(this, (String)item.getValue("Alias"), false));
        row.addCell(aliasColumn, new StringCell((String)item.getValue("Alias"), item, true));
        row.addCell(addressColumn, new StringCell((String)item.getValue("Address"), item, false));

        final boolean hide =((Boolean)item.getValue("Hide")).booleanValue();
        row.addCell(hideColumn, new StringCell(hide ? "true" : "false", item, false));
      }

    }
  }

  protected int calculateTotalSize() {
    return (aliasesList != null) ? aliasesList.size() : 0;
  }

  protected int onCellSelected(Column column, Row row, DataCell cell) {
    selectedAlias = (DataItem)cell.getObject();
    addToTemporaryContent("aliasesList", null);
    return RESULT_EDIT;
  }

  public DataItem getSelectedAlias() {
    return selectedAlias;
  }



  // ===================================================================================================================

  public String getMbDelete() {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete) {
    this.mbDelete = mbDelete;
  }

  public String getMbAdd() {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd) {
    this.mbAdd = mbAdd;
  }

  public String[] getAliases() {
    return aliases;
  }

  public void setAliases(String[] aliases) {
    this.aliases = aliases;
  }

  public String[] getAddresses() {
    return addresses;
  }

  public void setAddresses(String[] addresses) {
    this.addresses = addresses;
  }

  public byte getHideByte() {
    return hide;
  }

  public String getHide() {
    return String.valueOf(hide);
  }

  public void setHide(String hide) {
    try {
      this.hide = Byte.decode(hide).byteValue();
    } catch (NumberFormatException e) {
      this.hide = AliasFilter.HIDE_UNKNOWN;
    }
  }

  public String getMbQuery() {
    return mbQuery;
  }

  public void setMbQuery(String mbApply) {
    this.mbQuery = mbApply;
  }

  public boolean isInitialized() {
    return initialized;
  }

  protected String getBeanId() {
    return "ru.novosoft.smsc.jsp.smsc.aliases.Index";
  }

  public String getMbClear() {
    return mbClear;
  }

  public void setMbClear(String mbClear) {
    this.mbClear = mbClear;
  }
}


