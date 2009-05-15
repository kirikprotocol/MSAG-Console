package ru.novosoft.smsc.infosme.backend.tables.stat;

import ru.novosoft.smsc.jsp.util.helper.statictable.*;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.StringCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.column.TextColumn;
import ru.novosoft.smsc.util.LocaleMessages;

import java.util.*;
import java.io.Writer;
import java.io.IOException;
import java.text.SimpleDateFormat;

/**
 * User: artem
 * Date: 23.04.2009
 */
public class TaskStatTableHelper extends PagedStaticTableHelper {

  private final TextColumn task = new TextColumn("taskName", "infosme.label.task", true);
  private final TextColumn activityStart = new TextColumn("activityStart", "infosme.label.activityStart", true);
  private final TextColumn activityEnd = new TextColumn("activityEnd", "infosme.label.activityEnd", true);
  private final TextColumn generated = new TextColumn("generated", "infosme.label.generated", true);
  private final TextColumn delivered = new TextColumn("delivered", "infosme.label.delivered", true);
  private final TextColumn retried = new TextColumn("retried", "infosme.label.retried", true);
  private final TextColumn failed = new TextColumn("failed", "infosme.label.failed", true);

  private final StatisticsDataSource ds;
  private final StatQuery filter;

  private int size;
  private int totalGenerated, totalDelivered, totalRetried, totalFailed;

  public TaskStatTableHelper(String uid, StatisticsDataSource ds, StatQuery filter) {
    super(uid, false);

    this.ds = ds;
    this.filter = filter;

    addColumn(task);
    addColumn(activityStart);
    addColumn(activityEnd);
    addColumn(generated);
    addColumn(delivered);
    addColumn(retried);
    addColumn(failed);

    setSort(task.getId(), OrderType.ASC);
  }

  private Set getStatsMap() {
    final Map results = new HashMap(100);

    ds.visit(new StatVisitor() {
      public boolean visit(StatisticDataItem item) {
        String taskName = (String) item.getValue("taskName");
        Stat stat = (Stat) results.get(taskName);
        if (stat == null) {
          stat = new Stat();
          stat.taskName = taskName;
          stat.activityStart = (Date)item.getValue("period");
          results.put(taskName, stat);
        }
        stat.delivered += ((Integer) item.getValue("delivered")).intValue();
        stat.generated += ((Integer) item.getValue("generated")).intValue();
        stat.retried += ((Integer) item.getValue("retried")).intValue();
        stat.failed += ((Integer) item.getValue("failed")).intValue();
        stat.activityEnd = (Date)item.getValue("period");
        return true;
      }
    }, filter);

    TreeSet sortedResults = new TreeSet(new StatComparator());
    sortedResults.addAll(results.values());

    return sortedResults;
  }

  protected void fillTable(int start, int size) throws TableHelperException {

    Set sortedResults = getStatsMap();
    this.size = sortedResults.size();

    SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yy HH:mm");

    int i = 0;

    totalDelivered = totalGenerated = totalRetried = totalFailed = 0;
    for (Iterator iter = sortedResults.iterator(); iter.hasNext(); i++) {
      Stat stat = (Stat) iter.next();
      String taskName = stat.taskName;

      if (i >= start && i < start + size) {
        Row r = createNewRow();
        r.addCell(task, new StringCell(taskName, taskName, false));
        r.addCell(activityStart, new StringCell(taskName, sdf.format(stat.activityStart), false));
        r.addCell(activityEnd, new StringCell(taskName, sdf.format(stat.activityEnd), false));
        r.addCell(generated, new StringCell(taskName, String.valueOf(stat.generated), false));
        r.addCell(delivered, new StringCell(taskName, String.valueOf(stat.delivered), false));
        r.addCell(retried, new StringCell(taskName, String.valueOf(stat.retried), false));
        r.addCell(failed, new StringCell(taskName, String.valueOf(stat.failed), false));
      }

      totalGenerated += stat.generated;
      totalDelivered += stat.delivered;
      totalRetried += stat.retried;
      totalFailed += stat.failed;
    }
  }

  public void exportCsv(Writer os) throws IOException {
    Set sortedResults = getStatsMap();
    SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yy HH:mm");
    Locale csvLocale = locale;
    if (csvLocale == null)
      csvLocale = new Locale(LocaleMessages.DEFAULT_PREFERRED_LANGUAGE);
    ResourceBundle bundle = ResourceBundle.getBundle(LocaleMessages.SMSC_BUNDLE_NAME, csvLocale);
    // Export header
    os.write(bundle.getString(task.getTitle()));
    os.write(";");
    os.write(bundle.getString(activityStart.getTitle()));
    os.write(";");
    os.write(bundle.getString(activityEnd.getTitle()));
    os.write(";");
    os.write(bundle.getString(generated.getTitle()));
    os.write(";");
    os.write(bundle.getString(delivered.getTitle()));
    os.write(";");
    os.write(bundle.getString(retried.getTitle()));
    os.write(";");
    os.write(bundle.getString(failed.getTitle()));
    os.write("\r\n");
    // Export table
    for (Iterator iter = sortedResults.iterator(); iter.hasNext();) {
      Stat stat = (Stat) iter.next();
      os.write(stat.taskName);
      os.write(";");
      os.write(sdf.format(stat.activityStart));
      os.write(";");
      os.write(sdf.format(stat.activityEnd));      
      os.write(";");
      os.write(String.valueOf(stat.generated));
      os.write(";");
      os.write(String.valueOf(stat.delivered));
      os.write(";");
      os.write(String.valueOf(stat.retried));
      os.write(";");
      os.write(String.valueOf(stat.failed));
      os.write("\r\n");
    }
    // Export total data
    os.write(bundle.getString("infosme.label.total"));
    os.write(";");
    os.write(";");
    os.write(";");
    os.write(String.valueOf(totalGenerated));
    os.write(";");
    os.write(String.valueOf(totalDelivered));
    os.write(";");
    os.write(String.valueOf(totalRetried));
    os.write(";");
    os.write(String.valueOf(totalFailed));
    os.write("\r\n");
  }

  protected int calculateTotalSize() throws TableHelperException {
    return size;
  }

  public int getTotalGenerated() {
    return totalGenerated;
  }

  public int getTotalDelivered() {
    return totalDelivered;
  }

  public int getTotalRetried() {
    return totalRetried;
  }

  public int getTotalFailed() {
    return totalFailed;
  }

  private static class Stat {
    private String taskName;
    private Date activityStart;
    private Date activityEnd;
    private int generated;
    private int delivered;
    private int retried;
    private int failed;
  }

  private class StatComparator implements Comparator {
    public int compare(Object o1, Object o2) {
      Stat s1 = (Stat)o1;
      Stat s2 = (Stat)o2;
      
      if (getSortOrder() == null || getSortOrder().length == 0)
        return 0;

      SortOrderElement sortOrder = getSortOrder()[0];

      int result;
      if (sortOrder != null && sortOrder.getColumnId().equals(generated.getId())) {
        result = s1.generated > s2.generated ? 1 : -1;
      } else if (sortOrder != null && sortOrder.getColumnId().equals(delivered.getId())) {
        result = s1.delivered > s2.delivered ? 1 : -1;
      } else if (sortOrder != null && sortOrder.getColumnId().equals(retried.getId())) {
        result = s1.retried > s2.retried ? 1 : -1;
      } else if (sortOrder != null && sortOrder.getColumnId().equals(failed.getId())) {
        result = s1.failed > s2.failed ? 1 : -1;
      } else if (sortOrder != null && sortOrder.getColumnId().equals(activityStart.getId())) {
        result = s1.activityStart.compareTo(s2.activityStart);
        if (result == 0) result = -1;
      } else if (sortOrder != null && sortOrder.getColumnId().equals(activityEnd.getId())) {
        result = s1.activityEnd.compareTo(s2.activityEnd);
        if (result == 0) result = -1;
      } else
        result = s1.taskName.compareTo(s2.taskName);

      return sortOrder == null || sortOrder.getOrderType() == OrderType.ASC ? result : -result;
    }
  }
}
