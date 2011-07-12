package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.components.data_table.model.LoadListener;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
class SimpleErrorStatsStrategy implements ErrorStatsStrategy{

  private Map<Integer, SimpleRow> rowsMap = new HashMap<Integer, SimpleRow>(10);

  private final InformerStrategy informerStrategy;

  SimpleErrorStatsStrategy(InformerStrategy informerStrategy) {
    this.informerStrategy = informerStrategy;
  }

  public void execute(Delivery delivery, User user, final LoadListener listener) throws AdminException {
    rowsMap.clear();
    MessageFilter filter = new MessageFilter(delivery.getId(),delivery.getCreateDate(), delivery.getEndDate() == null ? new Date() : delivery.getEndDate());
    filter.setStates(MessageState.Delivered, MessageState.Expired, MessageState.Failed);
    listener.setTotal(informerStrategy.countMessages(user.getLogin(), filter)+1);
    informerStrategy.getMessagesStates(user.getLogin(), filter, 2000, new Visitor<Message>() {
      @Override
      public boolean visit(Message value) throws AdminException {
        listener.setCurrent(listener.getCurrent()+1);
        int error = value.getErrorCode() == null ? 0 : value.getErrorCode();
        SimpleRow r = rowsMap.get(error);
        if(r == null) {
          r = new SimpleRow(error);
          rowsMap.put(error, r);
        }
        r.incrementCount();
        return true;
      }
    });
  }

  @Override
  public List getRows(int startPos, int count, DataTableSortOrder sortOrder) {
    List<SimpleRow> rs = new ArrayList<SimpleRow>(rowsMap.values());

    Collections.sort(rs, getComparator(sortOrder));

    List<SimpleRow> result = new LinkedList<SimpleRow>();
    for (Iterator<SimpleRow> i = rs.iterator(); i.hasNext() && count > 0;) {
      SimpleRow r = i.next();
      if (--startPos < 0) {
        result.add(r);
        count--;
      }
    }
    return result;
  }

  private Comparator<SimpleRow> getComparator(final DataTableSortOrder order) {
    return new Comparator<SimpleRow>() {
      @Override
      public int compare(SimpleRow o1, SimpleRow o2) {
        int c = order == null ? 1 : order.isAsc() ? 1 : -1;
        if(order == null || order.getColumnId().equals("errorCode")) {
          return (o1.getErrorCode() < o2.getErrorCode() ? -1 : o1.getErrorCode() > o2.getErrorCode() ? 1 : 0)*c;
        }else {
          return (o1.getCount() < o2.getCount() ? -1 : o1.getCount() > o2.getCount() ? 1 : 0)*c;
        }
      }
    };
  }

  @Override
  public int getRowsCount() {
    return rowsMap.size();
  }

  @Override
  public void download(PrintWriter writer) throws IOException {
    writer.println("ERROR_CODE;COUNT");
    for(SimpleRow r : rowsMap.values()) {
      writer.println(StringEncoderDecoder.toCSVString(';', r.getErrorCode(), r.getCount()));
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
}
