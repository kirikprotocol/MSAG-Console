package ru.novosoft.smsc.web.controllers.sms_view;

import mobi.eyeline.util.jsf.components.data_table.model.DataTableModel;
import mobi.eyeline.util.jsf.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.operative_store.Message;
import ru.novosoft.smsc.admin.operative_store.MessageFilter;
import ru.novosoft.smsc.admin.operative_store.OperativeStoreProvider;
import ru.novosoft.smsc.admin.operative_store.ProgressObserver;
import ru.novosoft.smsc.util.Address;

import java.util.*;

/**
 * @author Artem Snopkov
 */
class GetSmsFromOperative implements GetSmsStrategy {

  private final OperativeStoreProvider provider;
  private final int smscInstancesNumber;

  public GetSmsFromOperative(OperativeStoreProvider provider, int smscInstancesNumber) {
    this.provider = provider;
    this.smscInstancesNumber = smscInstancesNumber;
  }

  public DataTableModel getSms(SmsQuery query, final GetSmsProgress progress) throws AdminException {
    MessageFilter f = new MessageFilter();
    f.setAbonentAddress(query.getAbonentAddress());
    f.setFromAddress(query.getFromAddress());
    f.setToAddress(query.getToAddress());

    f.setFromDate(query.getFromDate());
    f.setTillDate(query.getTillDate());
    f.setSmsId(query.getSmsId());
    f.setRouteId(query.getRouteId());

    f.setSmeId(query.getSmeId());
    f.setSrcSmeId(query.getSrcSmeId());
    f.setDstSmeId(query.getDstSmeId());

    progress.setTotal(smscInstancesNumber * 100);

    final Collection[] msgs = new Collection[smscInstancesNumber];

    int totalSize = 0;
    for (int i = 0; i < smscInstancesNumber; i++) {
      final int currentSmsc = i;
      msgs[i] = provider.getMessages(i, f, new ProgressObserver() {
        public void update(long current, long total) {
          progress.setCurrent((int) (currentSmsc * 100 + current / total * 100));
        }
      });
      if (msgs[i] != null)
        totalSize += msgs[i].size();
    }

    final int total = totalSize;

    return new DataTableModel() {
      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {
        SortedListHelper helper = new SortedListHelper(startPos + count, new Comparator<Message>() {
          public int compare(Message o1, Message o2) {
            if (sortOrder == null)
              return 0;
            int mul = (sortOrder.isAsc()) ? 1 : -1;
            try {
              return mul * o1.getLastTryTime().compareTo(o2.getLastTryTime());
            } catch (AdminException e) {
              return 0;
            }
          }
        });

        for (int i = 0; i < smscInstancesNumber; i++) {
          Collection<Message> messages = (Collection<Message>)msgs[i];
          if (messages != null)
            helper.addAll(messages);
        }

        List<Sms> result = new ArrayList<Sms>(count);
        int pos = 0;
        for (Iterator<Message> iter = helper.getList().iterator(); iter.hasNext();) {
          if (pos < startPos) {
            iter.next();
            pos++;
            continue;
          }
          result.add(new SmsImpl(iter.next()));
        }
        return result;
      }

      public int getRowsCount() {
        return total;
      }
    };
  }




  private static class SmsImpl implements Sms {

    private final Message m;

    public SmsImpl(Message m) {
      this.m = m;
    }

    public long getId() throws AdminException {
      return m.getId();
    }

    public Date getSubmitTime() throws AdminException {
      return m.getSubmitTime();
    }

    public Date getValidTime() throws AdminException {
      return m.getValidTime();
    }

    public Integer getAttempts() throws AdminException {
      return m.getAttempts();
    }

    public Integer getLastResult() throws AdminException {
      return m.getLastResult();
    }

    public Date getLastTryTime() throws AdminException {
      return m.getLastTryTime();
    }

    public Date getNextTryTime() throws AdminException {
      return m.getNextTryTime();
    }

    public Address getOriginatingAddress() throws AdminException {
      return m.getOriginatingAddress();
    }

    public Address getDestinationAddress() throws AdminException {
      return m.getDestinationAddress();
    }

    public Address getDealiasedDestinationAddress() throws AdminException {
      return m.getDealiasedDestinationAddress();
    }

    public Integer getMessageReference() throws AdminException {
      return m.getMessageReference();
    }

    public String getServiceType() throws AdminException {
      return m.getServiceType();
    }

    public Integer getDeliveryReport() throws AdminException {
      return m.getDeliveryReport();
    }

    public Integer getBillingRecord() throws AdminException {
      return m.getBillingRecord();
    }

    public RoutingInfo getOriginatingDescriptor() throws AdminException {
      ru.novosoft.smsc.admin.operative_store.RoutingInfo i = m.getOriginatingDescriptor();
      return new RoutingInfo(i.getImsi(), i.getMsc(), i.getSme());
    }

    public RoutingInfo getDestinationDescriptor() throws AdminException {
      ru.novosoft.smsc.admin.operative_store.RoutingInfo i = m.getDestinationDescriptor();
      return new RoutingInfo(i.getImsi(), i.getMsc(), i.getSme());
    }

    public String getRouteId() throws AdminException {
      return m.getRouteId();
    }

    public Integer getServiceId() throws AdminException {
      return m.getServiceId();
    }

    public Integer getPriority() throws AdminException {
      return m.getPriority();
    }

    public String getSrcSmeId() throws AdminException {
      return m.getSrcSmeId();
    }

    public String getDstSmeId() throws AdminException {
      return m.getDstSmeId();
    }

    public byte getArc() throws AdminException {
      return m.getArc();
    }

    public byte[] getBody() throws AdminException {
      return m.getBody();
    }

    public String getOriginalText() throws AdminException {
      return m.getOriginalText();
    }

    public String getText() throws AdminException {
      return m.getText();
    }

    public boolean isTextEncoded() throws AdminException {
      return m.isTextEncoded();
    }
  }



  
  private class SortedListHelper {

    private final LinkedList<Message> list;
    private final int size;
    private final Comparator<Message> comparator;

    SortedListHelper(int size, Comparator<Message> comparator) {
      this.comparator = comparator;
      this.list = new LinkedList<Message>();
      this.size = size;
    }

    public void add(Message message) {
      for (ListIterator<Message> iter = list.listIterator(); iter.hasNext();) {
        Message m = iter.next();
        int res = comparator.compare(m, message);
        if (res < 0) {
          iter.add(message);
          if (list.size() > size)
            list.removeLast();
          return;
        }
      }
      list.add(message);
      if (list.size() > size)
        list.removeLast();
    }

    public void addAll(Collection<Message> messages) {
      for (Message m : messages)
        add(m);
    }

    public List<Message> getList() {
      return list;
    }
  }
}
