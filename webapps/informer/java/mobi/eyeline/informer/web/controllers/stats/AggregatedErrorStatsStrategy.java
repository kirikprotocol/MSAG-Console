package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;

import java.io.IOException;
import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class AggregatedErrorStatsStrategy implements ErrorStatsStrategy{

  private final Map<String, AggregatedRow> rowMap = new HashMap<String, AggregatedRow>();

  private final PeriodCalc periodCalc;

  private final InformerStrategy informerStrategy;

  public AggregatedErrorStatsStrategy(GroupType groupType, InformerStrategy informerStrategy) {
    this.informerStrategy = informerStrategy;
    switch (groupType) {
      case DAY:
        periodCalc = new DayPeriodCalc();
        break;
      default:
        periodCalc = new HourPeriodCalc();
    }
  }

  public void execute(Delivery delivery, User user, final ProgressListener progressListener) throws Exception {
    rowMap.clear();
    MessageFilter filter = new MessageFilter(delivery.getId(), delivery.getCreateDate(), delivery.getEndDate() == null ? new java.util.Date() : delivery.getEndDate());
    filter.setStates(MessageState.Delivered, MessageState.Expired, MessageState.Failed);
    progressListener.setTotal(informerStrategy.countMessages(user.getLogin(), filter)+1);
    informerStrategy.getMessagesStates(user.getLogin(), filter, 2000, new Visitor<Message>() {
      @Override
      public boolean visit(Message value) throws AdminException {
        progressListener.incrementCurrent();
        int error = value.getErrorCode() == null ? 0 : value.getErrorCode();
        addRow(value.getDate(), error);
        return true;
      }
    });
  }

  @Override
  public List getRows(int startPos, int count, DataTableSortOrder sortOrder) {
    List<AggregatedRow> rs = new ArrayList<AggregatedRow>(rowMap.values());
    Collections.sort(rs, getComparator(sortOrder));
    List<AggregatedRow> result = new LinkedList<AggregatedRow>();
    for (Iterator<AggregatedRow> i = rs.iterator(); i.hasNext() && count > 0;) {
      AggregatedRow r = i.next();
      if (--startPos < 0) {
        result.add(r);
        count--;
      }
    }
    return result;
  }

  private Comparator<AggregatedRow> getComparator(final DataTableSortOrder order) {
    return new Comparator<AggregatedRow>() {
      @Override
      public int compare(AggregatedRow o1, AggregatedRow o2) {
        int c = order == null ? 1 : order.isAsc() ? 1 : -1;
        return (o1.getPeriod().compareTo(o2.getPeriod()))*c;
      }
    };
  }

  @Override
  public int getRowsCount() {
    return rowMap.size();
  }

  @Override
  public void download(PrintWriter writer) throws IOException {
    writer.println("PERIOD;ERROR_CODE;COUNT");
    for(AggregatedRow r : rowMap.values()) {
      for(SimpleRow sR : r.inner.values()) {
        writer.println(StringEncoderDecoder.toCSVString(';', r.getPeriod(), sR.getErrorCode(), sR.getCount()));
      }
    }
  }


  private void addRow(Date date, int errorCode) {
    String period = periodCalc.getPeriod(date);
    AggregatedRow row = rowMap.get(period);
    if(row  == null) {
      row = new AggregatedRow(period);
      rowMap.put(period, row);
    }
    row.addInner(errorCode);
  }


  public static class AggregatedRow {

    private String period;

    private int total;

    public AggregatedRow(String period) {
      this.period = period;
    }

    private Map<Integer, SimpleRow> inner = new TreeMap<Integer, SimpleRow>();

    public String getPeriod() {
      return period;
    }

    public Collection<SimpleRow> getInner() {
      return new ArrayList<SimpleRow>(inner.values());
    }

    public void addInner(int errorCode) {
      SimpleRow row = inner.get(errorCode);
      if(row == null) {
        row = new SimpleRow(errorCode);
        inner.put(errorCode, row);
      }
      row.incrementCount();
      total++;
    }

    public int getTotal() {
      return total;
    }
  }

  public static class SimpleRow {
    private final int errorCode;
    private int count;
    public SimpleRow(int errorCode) {
      this.errorCode = errorCode;
    }
    public int getCount() {
      return count;
    }
    public int getErrorCode() {
      return errorCode;
    }
    void incrementCount() {
      count++;
    }
  }

  public static enum GroupType {
    HOUR, DAY
  }

  private static interface PeriodCalc {
    String getPeriod(Date date);
  }

  private static class DayPeriodCalc implements PeriodCalc {
    private final SimpleDateFormat prefixFormat = new SimpleDateFormat("yyyy.MM.dd");
    @Override
    public String getPeriod(Date date) {
      Calendar c = Calendar.getInstance();
      c.setTime(date);
      c.set(Calendar.HOUR_OF_DAY, 0);
      c.set(Calendar.MINUTE, 0);
      c.set(Calendar.SECOND, 0);
      c.set(Calendar.MILLISECOND, 0);

      String part1 = prefixFormat.format(c.getTime());

      c.add(Calendar.DAY_OF_MONTH, 1);
      return part1;
    }
  }

  private static class HourPeriodCalc implements PeriodCalc {
    private final SimpleDateFormat prefixFormat = new SimpleDateFormat("yyyy.MM.dd HH:mm");
    private final SimpleDateFormat suffixFormat = new SimpleDateFormat("HH:mm");

    @Override
    public String getPeriod(Date date) {
      Calendar c = Calendar.getInstance();
      c.setTime(date);
      c.set(Calendar.MINUTE, 0);
      c.set(Calendar.SECOND, 0);
      c.set(Calendar.MILLISECOND, 0);

      String part1 = prefixFormat.format(c.getTime());

      c.add(Calendar.HOUR_OF_DAY, 1);
      String part2 = suffixFormat.format(c.getTime());

      return part1+" - "+part2;
    }
  }
}
