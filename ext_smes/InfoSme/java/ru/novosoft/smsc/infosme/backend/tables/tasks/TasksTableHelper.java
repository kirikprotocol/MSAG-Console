package ru.novosoft.smsc.infosme.backend.tables.tasks;

import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.jsp.util.helper.statictable.OrderType;
import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;
import ru.novosoft.smsc.jsp.util.helper.statictable.Row;
import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.CheckBoxCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.ImageCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.StringCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.column.TextColumn;
import ru.novosoft.smsc.jsp.util.helper.statictable.column.ImageColumn;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;

import javax.servlet.http.HttpServletRequest;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * User: artem
 * Date: 22.01.2009
 */
public class TasksTableHelper extends PagedStaticTableHelper {

  public static final int MODE_ADMIN = 0;
  public static final int MODE_ADMIN_ACTIVE = 1;
  public static final int MODE_MARKET = 2;

  private final ImageColumn checkColumn = new ImageColumn("checkColumn", "/images/ico16_checked_sa.gif", "", false, 1);
  private final TextColumn enabledColumn = new TextColumn("enabled", "On", true, 1);
  private final TextColumn nameColumn = new TextColumn("name", "Task", true, 20);
  private final TextColumn providerColumn = new TextColumn("provider", "Provider", true);
  private final TextColumn generatingColumn = new TextColumn("generating", "Generating", false);
  private final TextColumn processingColumn = new TextColumn("processing", "Processing", false);
  private final TextColumn priorityColumn = new TextColumn("priority", "Priority", true);
  private final TextColumn retryOnFailColumn = new TextColumn("retryOnFail", "RetryOnFail", true);
  private final TextColumn replaceMessageColumn = new TextColumn("replaceMessage", "Replace", true, 1);
  private final TextColumn trackIntegrityColumn = new TextColumn("trackIntegrity", "Integrity", true, 1);
  private final TextColumn startDateColumn = new TextColumn("startDate", "Start", true);
  private final TextColumn endDateColumn = new TextColumn("endDate", "End", true);
  private final TextColumn ownerColumn = new TextColumn("owner", "Owner", true);
  private final TextColumn idColumn = new TextColumn("id", "Id", true, 2);

  private final TaskDataSource tds;
  private TaskFilter filter;
  private boolean selectable;
  private String sortOrder = "name";
  private final User user;

  public TasksTableHelper(String uid, int mode, InfoSmeContext ctx, TaskFilter filter, boolean selectable, User user) {
    super(uid, false);
    tds = new TaskDataSource(ctx.getInfoSme(), ctx.getInfoSmeConfig());
    this.filter = filter;
    this.selectable = selectable;
    this.user = user;

    addColumn(checkColumn);
    addColumn(enabledColumn);
    addColumn(idColumn);
    addColumn(nameColumn);
    addColumn(startDateColumn);
    addColumn(endDateColumn);

    if (mode == MODE_MARKET) {
      addColumn(generatingColumn);
      addColumn(processingColumn);
    } else if (mode == MODE_ADMIN_ACTIVE) {
      addColumn(ownerColumn);
      addColumn(generatingColumn);
      addColumn(processingColumn);
      addColumn(priorityColumn);
      addColumn(trackIntegrityColumn);
    } else {
      addColumn(ownerColumn);
      addColumn(providerColumn);
      addColumn(generatingColumn);
      addColumn(processingColumn);
      addColumn(priorityColumn);
      addColumn(retryOnFailColumn);
//      addColumn(replaceMessageColumn);
//      addColumn(trackIntegrityColumn);
    }
  }

  public TaskFilter getFilter() {
    return filter;
  }

  public void setFilter(TaskFilter filter) {
    this.filter = filter;
  }

  private void buildSortOrder() {
    SortOrderElement[] sortOrderElements = getSortOrder();
    if((sortOrderElements!=null)&&(sortOrderElements.length>0)) {
      SortOrderElement element = sortOrderElements[0];
      if(element!=null) {
        sortOrder=element.getColumnId();
        if(sortOrder!=null) {
          if(element.getOrderType()!= OrderType.ASC) {
            sortOrder= '-' +sortOrder;
          }
          return;
        }
      }
    }
    sortOrder="name";
  }

  protected void fillTable(int start, int size) throws TableHelperException {
    buildSortOrder();

    // Convert from user to local time
    if (filter != null) {
      if (filter.getStartDate() != null)
        filter.setStartDate(user.getLocalTime(filter.getStartDate()));

      if (filter.getEndDate() != null)
        filter.setEndDate(user.getLocalTime(filter.getEndDate()));
    }

    QueryResultSet rs = tds.query(new TaskQuery(filter, size, sortOrder, start));

    // Convert back from local to user time
    if (filter != null) {
      if (filter.getStartDate() != null)
        filter.setStartDate(user.getUserTime(filter.getStartDate()));

      if (filter.getEndDate() != null)
        filter.setEndDate(user.getUserTime(filter.getEndDate()));
    }

    SimpleDateFormat df = new SimpleDateFormat("dd.MM.yy HH:mm");

    clear();
    for (int i=0; i < size && i < rs.size(); i++) {
      TaskDataItem t = (TaskDataItem)rs.get(i);
      final Row row = createNewRow();
      row.addCell(checkColumn, new CheckBoxCell("chb" + t.getId(), false));
      row.addCell(enabledColumn, new ImageCell(t.getId(), t.isEnabled() ? "/images/ic_checked.gif" : null, false));
      row.addCell(idColumn, new StringCell(t.getId(), t.getId(), false));
      row.addCell(nameColumn, new StringCell(t.getId(), t.getName(), selectable));
      String startDate = t.getStartDate() == null ? "" : df.format(user.getUserTime(t.getStartDate()));
      row.addCell(startDateColumn, new StringCell(t.getId(), startDate, false));
      String endDate = t.getEndDate() == null ? "" : df.format(user.getUserTime(t.getEndDate()));
      row.addCell(endDateColumn, new StringCell(t.getId(), endDate, false));
      row.addCell(generatingColumn, new ImageCell(t.getId(), t.isGenerating() ? "/images/ic_running.gif" : "/images/ic_stopped.gif", false));
      row.addCell(processingColumn, new ImageCell(t.getId(), t.isProcessing() ? "/images/ic_running.gif" : "/images/ic_stopped.gif", false));
      row.addCell(ownerColumn, new StringCell(t.getId(), t.getOwner(), false));
      row.addCell(priorityColumn, new StringCell(t.getId(), String.valueOf(t.getPriority()), false));
      row.addCell(trackIntegrityColumn, new ImageCell(t.getId(), t.isTrackIntegrity() ? "/images/ic_checked.gif" : null, false));
      row.addCell(retryOnFailColumn, new StringCell(t.getId(), t.getRetryPolicy(), false));
      row.addCell(replaceMessageColumn, new ImageCell(t.getId(), t.isReplaceMessage() ? "/images/ic_checked.gif" : null, false));
      row.addCell(providerColumn, new StringCell(t.getId(), t.getProvider(), false));
    }

    setTotalSize(rs.getTotalSize());
  }

  protected int calculateTotalSize() throws TableHelperException {
    return getTotalSize();
  }

  public List getSelectedTasksList(HttpServletRequest request) {
    final ArrayList result = new ArrayList();
    for (Iterator iter = request.getParameterMap().keySet().iterator(); iter.hasNext(); ) {
      final String paramName = (String)iter.next();
      if (paramName.startsWith("chb"))
        result.add(paramName.substring(3));
    }
    return result;
  }
}
