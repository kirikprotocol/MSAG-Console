package ru.novosoft.smsc.infosme.backend.tables.stat;

import ru.novosoft.smsc.jsp.util.helper.statictable.*;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.StringCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.column.TextColumn;

import java.util.*;
import java.io.OutputStream;
import java.io.Writer;
import java.io.IOException;

/**
 * User: artem
 * Date: 23.04.2009
 */
public class TaskStatTableHelper extends PagedStaticTableHelper {

  private final Column task = new TextColumn("taskName", "Task", true);
  private final Column generated = new TextColumn("generated", "Generated", true);
  private final Column delivered = new TextColumn("delivered", "Delivered", true);
  private final Column retried = new TextColumn("retried", "Retried", true);
  private final Column failed = new TextColumn("failed", "Failed", true);

  private final StatisticsDataSource ds;
  private final StatQuery filter;

  private int size;
  private int totalGenerated, totalDelivered, totalRetried, totalFailed;

  public TaskStatTableHelper(String uid, StatisticsDataSource ds, StatQuery filter) {
    super(uid, false);

    this.ds = ds;
    this.filter = filter;

    addColumn(task);
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
          results.put(taskName, stat);
        }
        stat.delivered += ((Integer) item.getValue("delivered")).intValue();
        stat.generated += ((Integer) item.getValue("generated")).intValue();
        stat.retried += ((Integer) item.getValue("retried")).intValue();
        stat.failed += ((Integer) item.getValue("failed")).intValue();
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

    int i = 0;

    totalDelivered = totalGenerated = totalRetried = totalFailed = 0;
    for (Iterator iter = sortedResults.iterator(); iter.hasNext(); i++) {
      Stat stat = (Stat) iter.next();
      String taskName = stat.taskName;

      if (i >= start && i < start + size) {
        Row r = createNewRow();
        r.addCell(task, new StringCell(taskName, taskName, false));
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
    for (Iterator iter = sortedResults.iterator(); iter.hasNext();) {
      Stat stat = (Stat) iter.next();
      os.write(stat.taskName);
      os.write(",");
      os.write(String.valueOf(stat.generated));
      os.write(",");
      os.write(String.valueOf(stat.delivered));
      os.write(",");
      os.write(String.valueOf(stat.retried));
      os.write(",");
      os.write(String.valueOf(stat.failed));
      os.write("\r\n");
    }    
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
      } else
        result = s1.taskName.compareTo(s2.taskName);

      return sortOrder == null || sortOrder.getOrderType() == OrderType.ASC ? result : -result;
    }
  }
}
