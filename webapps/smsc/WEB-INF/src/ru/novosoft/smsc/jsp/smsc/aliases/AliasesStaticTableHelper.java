package ru.novosoft.smsc.jsp.smsc.aliases;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.alias.AliasSet;
import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;
import ru.novosoft.smsc.jsp.util.helper.statictable.Row;
import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.CheckBoxCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.StringCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.column.TextColumn;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasDataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasQuery;

import javax.servlet.http.HttpServletRequest;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * User: artem
 * Date: 15.01.2007
 */
public class AliasesStaticTableHelper extends PagedStaticTableHelper {

  private final TextColumn checkColumn = new TextColumn("checkColumn", "", false);
  private final TextColumn aliasColumn = new TextColumn("aliasColumn", "common.sortmodes.alias", false, 45);
  private final TextColumn addressColumn = new TextColumn("addressColumn", "common.sortmodes.address", false, 45);
  private final TextColumn hideColumn = new TextColumn("hideColumn", "common.sortmodes.hide", false, 5);

  private AliasFilter filter;
  private AliasSet aliasSet;
  private boolean editAllowed;
  private int maxTotalSize;

  private int totalSize = 0;

  public AliasesStaticTableHelper(String uid) {
    super(uid, false);

    addColumn(checkColumn);
    addColumn(aliasColumn);
    addColumn(addressColumn);
    addColumn(hideColumn);
  }

  protected void fillTable(int start, int size) throws TableHelperException {
    try {

      final QueryResultSet aliasesList = aliasSet.query(new AliasQuery(maxTotalSize, filter, AliasDataSource.ALIAS_FIELD, 0));

      for (int i = start; i < aliasesList.size() && i < start + size; i++) {
        final DataItem item = (DataItem) aliasesList.get(i);

        final Row row = createNewRow();
        final String aliasName = (String) item.getValue("Alias");
        row.addCell(checkColumn, new CheckBoxCell("chb" + aliasName, false));
        row.addCell(aliasColumn, new StringCell(aliasName, aliasName, editAllowed));
        row.addCell(addressColumn, new StringCell(aliasName, (String) item.getValue("Address"), false));

        final boolean hide = ((Boolean) item.getValue("Hide")).booleanValue();
        row.addCell(hideColumn, new StringCell(aliasName, hide ? "true" : "false", false));
      }

      totalSize = aliasesList.size();

    } catch (AdminException e) {
      throw new TableHelperException(e);
    }
  }

  public List getSelectedAliasesList(HttpServletRequest request) {
    final ArrayList result = new ArrayList();
    for (Iterator iter = request.getParameterMap().keySet().iterator(); iter.hasNext(); ) {
      final String paramName = (String)iter.next();
      if (paramName.startsWith("chb"))
        result.add(paramName.substring(3));
    }
    return result;
  }

  protected int calculateTotalSize() throws TableHelperException {
    return totalSize;
  }

  public int getTotalSize() {
    return totalSize;
  }

  public AliasFilter getFilter() {
    return filter;
  }

  public void setFilter(AliasFilter filter) {
    this.filter = filter;
  }

  public AliasSet getAliasSet() {
    return aliasSet;
  }

  public void setAliasSet(AliasSet aliasSet) {
    this.aliasSet = aliasSet;
  }

  public boolean isEditAllowed() {
    return editAllowed;
  }

  public void setEditAllowed(boolean editAllowed) {
    this.editAllowed = editAllowed;
  }

  public int getMaxTotalSize() {
    return maxTotalSize;
  }

  public void setMaxTotalSize(int maxTotalSize) {
    this.maxTotalSize = maxTotalSize;
  }
}
