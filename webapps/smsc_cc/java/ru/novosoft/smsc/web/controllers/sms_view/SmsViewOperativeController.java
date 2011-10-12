package ru.novosoft.smsc.web.controllers.sms_view;

import mobi.eyeline.util.jsf.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.util.jsf.components.data_table.model.LoadListener;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.operative_store.Message;
import ru.novosoft.smsc.admin.operative_store.MessageFilter;
import ru.novosoft.smsc.admin.operative_store.ProgressObserver;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.web.WebContext;

import javax.faces.model.SelectItem;
import java.io.IOException;
import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class SmsViewOperativeController extends SmsViewController {

  private MessageFilter messageFilter = new MessageFilter();

  private MessageFilter oldFilter;

  private Collection[] msgs;

  private int totalSize;

  private int currentInstance = -1;

  private int smscInstancesNumber;

  private boolean init = false;

  public SmsViewOperativeController() {
    try {
      smscInstancesNumber = WebContext.getInstance().getSmscManager().getSettings().getSmscInstancesCount();
    } catch (AdminException e) {
      addError(e);
    }
    oldFilter = new MessageFilter(messageFilter);
  }

  public MessageFilter getMessageFilter() {
    return messageFilter;
  }

  public void setMessageFilter(MessageFilter messageFilter) {
    this.messageFilter = messageFilter;
  }

  public String apply() {
    init = true;
    if(!oldFilter.equals(messageFilter)) {
      loadingIsNeeded();
      oldFilter = new MessageFilter(messageFilter);
    }
    return null;
  }

  public boolean isInit() {
    return init;
  }

  public void setInit(boolean init) {
    this.init = init;
  }

  public String clear() {
    init = true;
    messageFilter = new MessageFilter();
    loadingIsNeeded();
    return null;
  }

  public List<SelectItem> getSmscInstances() {
    List<SelectItem> res = new ArrayList<SelectItem>(smscInstancesNumber+1);
    res.add(new SelectItem(-1, getLocalizedString("smsc.sms.view.smsc.instance.all")));
    for(int i=0; i < smscInstancesNumber; i++) {
      res.add(new SelectItem(i));
    }
    return res;
  }

  public int getCurrentInstance() {
    return currentInstance;
  }

  public void setCurrentInstance(int currentInstance) {
    this.currentInstance = currentInstance;
  }


  public int getSmscInstancesNumber() {
    return smscInstancesNumber;
  }

  public String getSmsId() {
    return messageFilter.getSmsId() == null ? null : messageFilter.getSmsId().toString();
  }

  public void setSmsId(String smsId) {
    messageFilter.setSmsId(smsId != null && (smsId = smsId.trim()).length() != 0 ? Long.parseLong(smsId) : null);
  }

  public String getLastResult() {
    return messageFilter.getLastResult() == null ? null : messageFilter.getLastResult().toString();
  }

  public void setLastResult(String lastResult) {
    messageFilter.setLastResult(lastResult == null || (lastResult = lastResult.trim()).length() == 0 ? null : Integer.parseInt(lastResult));
  }


  protected void _load(Locale locale, final LoadListener loadListener) throws AdminException {
    int smscInstancesNumber = getSmscInstancesNumber();
    loadListener.setTotal(smscInstancesNumber * 100);

    msgs = new Collection[smscInstancesNumber];

    totalSize = 0;
    for (int i = 0; i < smscInstancesNumber; i++) {
      final int currentSmsc = i;
      msgs[i] = wcontext.getOperativeStoreProvider().getMessages(i, messageFilter, new ProgressObserver() {
        public void update(long current, long total) {
          loadListener.setCurrent((int) (currentSmsc * 100 + current / total * 100));
        }
      });
      if (msgs[i] != null)
        totalSize += msgs[i].size();
    }
  }

  protected List getRows(int startPos, int count, DataTableSortOrder dataTableSortOrder)  {
    final int _instance = currentInstance;
    if(msgs == null || _instance >= msgs.length || (_instance != -1 && msgs[_instance] == null) ) {
      return Collections.emptyList();
    }
    List<Message> result = new LinkedList<Message>();
    List<Message> records;
    if(_instance == -1) {
      records = new ArrayList<Message>(totalSize);
      for(Collection<Message> c : msgs)  {
        records.addAll(c);
      }
    }else {
      records = new ArrayList<Message>(msgs[_instance]);
    }
    Collections.sort(records, getComparator(dataTableSortOrder));
    for (int i = startPos; i < Math.min(records.size(), startPos + count); i++) {
      result.add(records.get(i));
    }
    return result;
  }

  protected int getRowsCount() {
    final int _instance = currentInstance;
    if(msgs == null || _instance >= msgs.length || (_instance != -1 &&  msgs[_instance] == null)) {
      return 0;
    }
    return  _instance == -1 ? totalSize : msgs[_instance].size();
  }

  private static Comparator<Message> getComparator(final DataTableSortOrder sortOrder) {
    return new Comparator<Message>() {
      public int compare(Message o1, Message o2) {
        int preres = sortOrder == null || sortOrder.isAsc() ? 1 : -1;
        try{
          if(sortOrder == null || sortOrder.getColumnId().equals("id")) {
            preres *= o1.getId() < o2.getId() ? -1 : o1.getId() == o2.getId() ? 0 : 1;
          }else if(sortOrder.getColumnId().equals("sendDate")) {
            preres *= o1.getSubmitTime().compareTo(o2.getSubmitTime());
          }else if(sortOrder.getColumnId().equals("lastDate")) {
            preres *= o1.getLastTryTime().compareTo(o2.getLastTryTime());
          }else if(sortOrder.getColumnId().equals("from")) {
            preres *= o1.getOriginatingAddress().compareTo(o2.getOriginatingAddress());
          }else if(sortOrder.getColumnId().equals("to")) {
            preres *= o1.getDestinationAddress().compareTo(o2.getDestinationAddress());
          }else if(sortOrder.getColumnId().equals("status")) {
            preres *= o1.getStatus().compareTo(o2.getStatus());
          }
        }catch (AdminException e) {
          logger.error(e, e);
        }
        return preres;
      }
    };
  }

  @SuppressWarnings({"unchecked"})
  @Override
  protected void _download(PrintWriter writer) throws IOException {
    int currentInstance = getCurrentInstance();
    if(!isLoaded() || msgs == null || currentInstance>= msgs.length || (currentInstance != -1 && msgs[currentInstance] == null)) {
      return;
    }
    SimpleDateFormat dateFormatter = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss", getLocale());
    try{
      if(currentInstance == -1) {
        for(Collection<Message> c : msgs) {
          for (Message row : c) {
            appendRow(dateFormatter, writer, row);
          }
        }
      }else {
        for (Message row : (Collection<Message>)msgs[currentInstance]) {
          appendRow(dateFormatter, writer, row);
        }
      }
    }catch (AdminException e) {
      addError(e);
    }
  }

  private static final char TAB = '\t';
  private static final String LINE_SEP = "\r\n";
  private static final String SPACE = " ";

  private void appendRow(SimpleDateFormat dateFormatter, PrintWriter writer, Message row) throws AdminException {
    String id = Long.toString(row.getId());
    String submit = row.getSubmitTime() != null ? StringEncoderDecoder.encode(dateFormatter.format(row.getSubmitTime())) : SPACE;
    String valid = row.getValidTime() != null ? StringEncoderDecoder.encode(dateFormatter.format(row.getValidTime())) : SPACE;
    String last = row.getLastTryTime() != null ? StringEncoderDecoder.encode(dateFormatter.format(row.getLastTryTime())) : SPACE;
    String next = row.getNextTryTime() != null ? StringEncoderDecoder.encode(dateFormatter.format(row.getNextTryTime())) : SPACE;
    Address source = row.getOriginatingAddress();
    Address dest = row.getDestinationAddress();
    String route = row.getRouteId();
    Message.Status status = row.getStatus();
    //id + "\t" + submit + "\t" + valid + "\t" + last + "\t" + next + "\t" + source + "\t" + dest + "\t" + route + "\t" + status + "\t";
    writer.append(id).append(TAB).append(submit).append(TAB).append(valid).append(TAB).append(last).append(TAB).append(next).append(TAB);
    writer.append(source.getSimpleAddress()).append(TAB).append(dest.getSimpleAddress()).append(TAB).append(route).append(TAB).append(status.toString()).append(TAB);
    if (row.getOriginalText() != null && isAllowToShowSmsText(row)) {
      writer.append(row.getOriginalText());
    }
    writer.append(LINE_SEP);
  }

  private static final String MAP_PROXY = "MAP_PROXY";
  private static final String SPECIAL_ROLE1 = "smsView_smstext_p2p";
  private static final String SPECIAL_ROLE2 = "smsView_smstext_content";

  protected boolean isAllowToShowSmsText(Message row) throws AdminException{
    if (row.getSrcSmeId().equalsIgnoreCase(MAP_PROXY) && row.getDstSmeId().equals(MAP_PROXY)) {
      return isUserhasRole(SPECIAL_ROLE1);
    }else {
      return isUserhasRole(SPECIAL_ROLE2);
    }
  }
}
