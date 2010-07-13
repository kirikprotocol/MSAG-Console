package ru.novosoft.smsc.infosme.backend.tables.tasks;

import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;
import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;
import ru.novosoft.smsc.jsp.util.helper.statictable.Row;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.StringCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.ImageCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.column.TextColumn;
import ru.novosoft.smsc.jsp.SMSCAppContextImpl;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageDataSource;
import ru.novosoft.smsc.infosme.backend.tables.stat.StatisticsDataSource;
import ru.novosoft.smsc.infosme.backend.tables.stat.StatVisitor;
import ru.novosoft.smsc.infosme.backend.tables.stat.StatQuery;
import ru.novosoft.smsc.infosme.backend.tables.stat.StatisticDataItem;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.infosme.backend.config.InfoSmeConfig;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.LocaleMessages;
import ru.novosoft.util.jsp.AppContextImpl;

import java.util.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;

/**
 * User: artem
 * Date: 20.04.2009
 */
public class TaskWeeklyTableHelper extends PagedStaticTableHelper {

  private static final long WEEK_LEN_IN_MILLIS = 3600 * 1000 * 24 * 7;

  private final TextColumn monday = new TextColumn("Monday", "Mon");
  private final TextColumn tuesday = new TextColumn("Tuesday", "Tue");
  private final TextColumn wednesday = new TextColumn("Wednesday", "Wed");
  private final TextColumn thursday = new TextColumn("Thursday", "Thu");
  private final TextColumn friday = new TextColumn("Friday", "Fri");
  private final TextColumn saturday = new TextColumn("Saturday", "Sat");
  private final TextColumn sunday = new TextColumn("Sunday", "Sun");

  private final InfoSmeConfig config;

  private int total;
  private final Date minWeekStart;
  private final HashMap taskActivities;
  private final StatisticsDataSource sds;

  public TaskWeeklyTableHelper(String uid, InfoSmeContext ctx, TaskFilter filter) throws AdminException {
    super(uid, false);
    this.config = ctx.getInfoSmeConfig();

    addColumn(monday);
    addColumn(tuesday);
    addColumn(wednesday);
    addColumn(thursday);
    addColumn(friday);
    addColumn(saturday);
    addColumn(sunday);

    taskActivities = new HashMap(100);
    final Date[] minAndMax = new Date[]{null, null};

    TaskDataSource tds = new TaskDataSource(ctx.getInfoSme(), ctx.getInfoSmeConfig());
    String serviceFolder = ctx.getAppContext().getHostsManager().getServiceInfo("InfoSme").getServiceFolder().getAbsolutePath();
    String msgStoreDir = ctx.getInfoSmeConfig().getStoreLocation();
    if (msgStoreDir.length() > 0 && msgStoreDir.charAt(0) != '/')
      msgStoreDir = serviceFolder + '/' + msgStoreDir;
    String statStoreDir = ctx.getInfoSmeConfig().getStatStoreLocation();
    if (statStoreDir.length() > 0 && statStoreDir.charAt(0) != '/')
      statStoreDir = serviceFolder + '/' + statStoreDir;
    final MessageDataSource mds = new MessageDataSource(ctx.getInfoSmeConfig());
    sds = new StatisticsDataSource(statStoreDir);

    // Fill task activities, calculate min and max dates
    try {
      tds.visit(new TaskVisitor() {
        public boolean visit(TaskDataItem t) {
          try {
            SortedSet dates = mds.getTaskActivityDates(t.getId());
            if (!dates.isEmpty()) {
              Date first = (Date)dates.first();
              Date last = (Date)dates.last();
              if (minAndMax[0] == null || minAndMax[0].after(first))
                minAndMax[0] = first;
              if (minAndMax[1] == null || minAndMax[1].before(last))
                minAndMax[1] = last;
              taskActivities.put(t.getId(), dates);
            }
          } catch (ParseException e) {
            e.printStackTrace();
            return false;
          }
          return true;
        }
      }, filter);
    } catch (AdminException e) {
      throw new AdminException(e.getMessage(), e);
    }

    Date thisWeekStart = getWeekStart(new Date());
    minWeekStart = getWeekStart(minAndMax[0]);
    Date maxWeekStart = getWeekStart(minAndMax[1]);

    long d1 = (thisWeekStart.getTime() - minWeekStart.getTime()) / WEEK_LEN_IN_MILLIS + 1;
    if (d1 < 0)
      d1 = 0;
    long d2 = (maxWeekStart.getTime() - thisWeekStart.getTime()) / WEEK_LEN_IN_MILLIS + 1;
    if (d2 < 0)
      d2 = 0;
    total = (int)(d2 + d1);

    setStartPosition((int)d1);
    setPageSize(1);
    setMaxRows(total);
  }

  private static Date getWeekStart(Date date) {
    final Calendar startDate = Calendar.getInstance();
    startDate.setTime(date);
    startDate.set(Calendar.DAY_OF_WEEK, Calendar.MONDAY);
    startDate.set(Calendar.HOUR_OF_DAY, 0);
    startDate.set(Calendar.MINUTE, 0);
    startDate.set(Calendar.SECOND, 0);
    startDate.set(Calendar.MILLISECOND, 0);
    return startDate.getTime();
  }

  private void setColumnNames(Date startDate) {
    final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM");

    if (locale == null)
        locale = new Locale(LocaleMessages.DEFAULT_PREFERRED_LANGUAGE);
    ResourceBundle b = ResourceBundle.getBundle(LocaleMessages.SMSC_BUNDLE_NAME, locale);

    Calendar st = Calendar.getInstance();
    st.setTime(startDate);
    monday.setTitle(b.getString("infosme.label.monday.short") + ", " + sdf.format(st.getTime()));
    st.set(Calendar.DATE, st.get(Calendar.DATE) + 1);
    tuesday.setTitle(b.getString("infosme.label.tuesday.short") + ", " + sdf.format(st.getTime()));
    st.set(Calendar.DATE, st.get(Calendar.DATE) + 1);
    wednesday.setTitle(b.getString("infosme.label.wednesday.short") + ", " + sdf.format(st.getTime()));
    st.set(Calendar.DATE, st.get(Calendar.DATE) + 1);
    thursday.setTitle(b.getString("infosme.label.thursday.short") + ", " + sdf.format(st.getTime()));
    st.set(Calendar.DATE, st.get(Calendar.DATE) + 1);
    friday.setTitle(b.getString("infosme.label.friday.short") + ", " + sdf.format(st.getTime()));
    st.set(Calendar.DATE, st.get(Calendar.DATE) + 1);
    saturday.setTitle(b.getString("infosme.label.saturday.short") + ", " + sdf.format(st.getTime()));
    st.set(Calendar.DATE, st.get(Calendar.DATE) + 1);
    sunday.setTitle(b.getString("infosme.label.sunday.short") + ", " + sdf.format(st.getTime()));
  }

  private void fillRows(WeekDay[] weekDays) {
    int maxRows = 0;
    for (int i = 0; i < weekDays.length; i++)
      if (weekDays[i].tasks.size() > maxRows)
        maxRows = weekDays[i].tasks.size();

    for (int i = 0; i < maxRows; i++) {
      Row row = createNewRow();
      addToColumn(row, monday, i, 0, weekDays);
      addToColumn(row, tuesday, i, 1, weekDays);
      addToColumn(row, wednesday, i, 2, weekDays);
      addToColumn(row, thursday, i, 3, weekDays);
      addToColumn(row, friday, i, 4, weekDays);
      addToColumn(row, saturday, i, 5, weekDays);
      addToColumn(row, sunday, i, 6, weekDays);
    }
  }

  protected void fillTable(int start, int size) throws TableHelperException {
    Date startDate = getWeekStart(new Date(minWeekStart.getTime() + ((long)start) * WEEK_LEN_IN_MILLIS));

    setColumnNames(startDate);

    final WeekDay[] weekDays = new WeekDay[7];
    for (int i = 0; i < weekDays.length; i++)
      weekDays[i] = new WeekDay();

    final long now = System.currentTimeMillis();

    for (Iterator iter = taskActivities.entrySet().iterator(); iter.hasNext();) {
      Map.Entry e = (Map.Entry)iter.next();

      String taskId = (String)e.getKey();
      SortedSet dates = (SortedSet)e.getValue();

      long startTime = startDate.getTime();
      for (int i = 0; i < 7; i++) {
        if (dates.contains(new Date(startTime))) {
          Task t = config.getTask(taskId);
          if (startTime > now) {
            boolean active = t.getStartDate().getTime() <= (startTime + 3600 * 24 * 1000) && (t.getEndDate() == null || t.getEndDate().getTime() >= startTime);
            active = active && t.getActiveWeekDays().contains(Task.WEEK_DAYS[i]);
            if (active && t.isEnabled())
              weekDays[i].tasks.add(t);
          } else
            weekDays[i].tasks.add(t);
        }
        startTime += 3600 * 24 * 1000;
      }
    }

    fillRows(weekDays);
  }

  private Map getTasksActivities(Date date) {
    StatQuery q = new StatQuery();
    q.setFromDate(date);
    q.setTillDate(new Date(date.getTime() + 3600 * 24 * 1000));

    final Map result = new HashMap();
    sds.visit(new StatVisitor() {
      public boolean visit(StatisticDataItem item) {
        String id = (String)item.getValue("taskId");
        TaskActivity a = (TaskActivity)result.get(id);
        if (a == null) {
          a = new TaskActivity();
          result.put(id, a);
        }
        a.delivered += ((Integer)item.getValue("delivered")).intValue();
        a.failed += ((Integer)item.getValue("failed")).intValue();
        return true;
      }
    }, q);
    return result;
  }

  private static void addToColumn(Row row, TextColumn column, int rowNumber, int dayOfWeek, WeekDay[] weekDays) {
    Task t = rowNumber < weekDays[dayOfWeek].tasks.size() ? (Task) weekDays[dayOfWeek].tasks.get(rowNumber) : null;
    row.addCell(column, new ImageCell(t == null ? "" : t.getId(), t != null && t.isEnabled() ? "/images/ic_checked.gif" : null, t == null ? "" : t.getName(), t != null));    
  }

  protected int calculateTotalSize() throws TableHelperException {
    return total;
  }

  private static class WeekDay {
    private List tasks = new ArrayList(30);
  }

  private static class TaskActivity {
    private int delivered;
    private int failed;
  }

}
