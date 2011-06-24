package mobi.eyeline.informer.web.controllers.monitoring;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.monitoring.MBean;
import mobi.eyeline.informer.admin.monitoring.MonitoringEvent;
import mobi.eyeline.informer.admin.monitoring.MonitoringEventsFilter;
import mobi.eyeline.informer.admin.monitoring.MonitoringJournal;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.model.SelectItem;
import java.io.IOException;
import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class MonitoringController extends InformerController {

  private String filterBySource;
  private Date filterByStartDate;
  private Date filterByEndDate;

  private boolean init = false;

  private boolean errorsOnly = false;

  private List<MonitoringEvent> records;

  public MonitoringController() {
    Calendar c = Calendar.getInstance();
    c.set(Calendar.HOUR,0);
    c.set(Calendar.MINUTE,0);
    c.set(Calendar.SECOND,0);
    c.set(Calendar.MILLISECOND,0);
    c.add(Calendar.DATE,-3);
    filterByStartDate = c.getTime();
    init = true;
  }

  public void clearFilter() {
    filterBySource = null;
    filterByStartDate = null;
    filterByEndDate = null;
    init = false;
    errorsOnly = false;
  }

  public void query() {
    records = null;
    init = true;
  }

  public boolean isErrorsOnly() {
    return errorsOnly;
  }

  public void setErrorsOnly(boolean errorsOnly) {
    this.errorsOnly = errorsOnly;
  }

  public String getFilterBySource() {
    return filterBySource;
  }

  public void setFilterBySource(String filterBySource) {
    this.filterBySource = filterBySource;
    if (this.filterBySource != null) {
      this.filterBySource = this.filterBySource.trim();
      if (this.filterBySource.length() == 0)
        this.filterBySource = null;
    }
  }

  public List<SelectItem> getUniqueSources() {
    List<SelectItem> result = new ArrayList<SelectItem>();
    result.add(new SelectItem("",""));
    for (MBean.Source s : MBean.Source.values()) {
      result.add(new SelectItem(s.toString(), s.toString()));
    }
    Collections.sort(result, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    return result;
  }

  public boolean isInit() {
    return init;
  }

  public void setInit(boolean init) {
    this.init = init;
  }

  public Date getFilterByStartDate() {
    return filterByStartDate;
  }

  public void setFilterByStartDate(Date filterByStartDate) {
    this.filterByStartDate = filterByStartDate;
  }

  public Date getFilterByEndDate() {
    return filterByEndDate;
  }

  public void setFilterByEndDate(Date filterByEndDate) {
    this.filterByEndDate = filterByEndDate;
  }

  private void loadRecords() throws AdminException {
    if (this.records == null) {
      MonitoringEventsFilter eventsFilter = new MonitoringEventsFilter();
      eventsFilter.setEndDate(filterByEndDate);
      eventsFilter.setStartDate(filterByStartDate);
      eventsFilter.setSource(filterBySource == null || filterBySource.length() == 0 ? null : MBean.Source.valueOf(filterBySource));

      final LinkedList<MonitoringEvent> records = new LinkedList<MonitoringEvent>();
      getConfig().getMonitoringEvents(eventsFilter, new MonitoringJournal.Visitor(){
        private static final int LIMIT = 1000;
        public boolean visit(MonitoringEvent r) {
          if (records.size() == LIMIT) {
            records.removeFirst();
          }
          records.addLast(r);
          return true;
        }
      });
      this.records = records;
      if(errorsOnly) {
        errorsOnly();
      }
    }
  }

  private void errorsOnly() {
    Collections.sort(records, new Comparator<MonitoringEvent>() {
      @Override
      public int compare(MonitoringEvent o1, MonitoringEvent o2) {
        return o1.getTime() > o2.getTime() ? -1 : o1.getTime() < o2.getTime() ? 1 : 0;
      }
    });
    Iterator<MonitoringEvent> i = records.iterator();
    Set<Index> indexes = new HashSet<Index>();
    while (i.hasNext()) {
      MonitoringEvent e = i.next();
      Index index = new Index(e.getAlarmId(), e.getSource());
      if(indexes.contains(index)) {
        i.remove();
      }else {
        indexes.add(index);
      }
    }
    i = records.iterator();
    while (i.hasNext()) {
      MonitoringEvent e = i.next();
      if(e.getSeverity() == MonitoringEvent.Severity.NORMAL) {
        i.remove();
      }
    }

  }

  private class Index {
    private String alarmId;
    private MBean.Source source;
    private Index(String alarmId, MBean.Source source) {
      this.alarmId = alarmId;
      this.source = source;
    }
    @Override
    public boolean equals(Object o) {
      if (this == o) return true;
      if (o == null || getClass() != o.getClass()) return false;
      Index index = (Index) o;
      if (alarmId != null ? !alarmId.equals(index.alarmId) : index.alarmId != null) return false;
      if (source != index.source) return false;
      return true;
    }
    @Override
    public int hashCode() {
      int result = alarmId != null ? alarmId.hashCode() : 0;
      result = 31 * result + (source != null ? source.hashCode() : 0);
      return result;
    }
  }

  public DataTableModel getRecords() {
    if (!init) {
      records = Collections.emptyList();
    } else {
      try {
        loadRecords();
      } catch (AdminException e) {
        addError(e);
        records = Collections.emptyList();
      }
    }

    return new DataTableModel() {

      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {

        // Сортируем записи
        final int mul = sortOrder == null ? -1 : sortOrder.isAsc() ? 1 : -1;
        Collections.sort(records, new Comparator<MonitoringEvent>() {
          public int compare(MonitoringEvent o1, MonitoringEvent o2) {
            if (sortOrder == null || sortOrder.getColumnId().equals("time")) {
              return o1.getTime() >= o2.getTime() ? mul : -mul;
            } else if (sortOrder.getColumnId().equals("sourceId")) {
              return mul * o1.getSource().compareTo(o2.getSource());
            } else if (sortOrder.getColumnId().equals("alarmId")) {
              return mul * o1.getAlarmId().compareTo(o2.getAlarmId());
            } else if (sortOrder.getColumnId().equals("severity")) {
              return mul * o1.getSeverity().compareTo(o2.getSeverity());
            }
            return 0;
          }
        });


        List<MonitoringEventRow> result = new ArrayList<MonitoringEventRow>(records.size());

        for (int i = startPos; i < Math.min(records.size(), startPos + count); i++) {
          MonitoringEvent r = records.get(i);
          MonitoringEventRow row = new MonitoringEventRow();
          row.setDate(new Date(r.getTime()));
          row.setSourceId(r.getSource().toString());
          row.setAlarmId(r.getAlarmId());
          row.setText(r.getText());
          row.setSeverity(r.getSeverity());
          result.add(row);
        }

        return result;
      }

      public int getRowsCount() {
        return records.size();
      }
    };
  }

  @Override
  protected void _download(PrintWriter writer) throws IOException {
    try {
      loadRecords();
    } catch (AdminException e) {
      addError(e);
      records = Collections.emptyList();
    }
    super._download(writer);
    SimpleDateFormat sdf = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
    for (MonitoringEvent ev : records) {
      Map<String, String> props = ev.getProps();
      List<Object> array = new ArrayList<Object>(5+props.size());
      array.add(sdf.format(new Date(ev.getTime())));
      array.add(ev.getSource());
      array.add(ev.getAlarmId());
      array.add(ev.getSeverity());
      array.add(ev.getText());

      for(Map.Entry<String, String> e : props.entrySet()) {
        array.add(e.getKey()+"="+e.getValue());
      }
      StringEncoderDecoder.toCSV(';', writer, array.toArray());
      writer.println();
    }
  }


  public class MonitoringEventRow {
    private Date date;
    private String sourceId;
    private String alarmId;
    private String text;
    private MonitoringEvent.Severity severity;

    public Date getDate() {
      return date;
    }

    public void setDate(Date date) {
      this.date = date;
    }

    public String getSourceId() {
      return sourceId;
    }

    public void setSourceId(String sourceId) {
      this.sourceId = sourceId;
    }

    public String getAlarmId() {
      return alarmId;
    }

    public void setAlarmId(String alarmId) {
      this.alarmId = alarmId;
    }

    public String getText() {
      return text;
    }

    public void setText(String text) {
      this.text = text;
    }

    public String getSeverity() {
      return severity.toString();
    }

    public String getStyle() {
      switch (severity) {
        case NORMAL: return "green";
        case MINOR: return "orange";
        case MAJOR: return "#a52a2a";
        default: return "red";
      }
    }

    public void setSeverity(MonitoringEvent.Severity severity) {
      this.severity = severity;
    }
  }
}
