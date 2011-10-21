package ru.novosoft.smsc.web.controllers.sms_view;

import mobi.eyeline.util.jsf.components.data_table.model.*;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.CCSms;
import ru.novosoft.smsc.admin.operative_store.Message;
import ru.novosoft.smsc.admin.operative_store.MessageFilter;
import ru.novosoft.smsc.admin.util.ProgressObserver;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.WebContext;

import javax.faces.event.ActionEvent;
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

  private Collection[] msgs;

  private int totalSize;

  private int currentInstance = -1;

  private int smscInstancesNumber;

  private boolean init = false;

  private LoadListener loadListener;

  private boolean loaded;

  private List<String> selected;

  public SmsViewOperativeController() {
    try {
      smscInstancesNumber = WebContext.getInstance().getSmscManager().getSettings().getSmscInstancesCount();
    } catch (AdminException e) {
      addError(e);
    }
  }

  public boolean isLoaded() {
    return loaded;
  }

  protected void loadingIsNeeded() {
    loaded = false;
    loadListener = null;
  }

  public MessageFilter getMessageFilter() {
    return messageFilter;
  }

  public void setMessageFilter(MessageFilter messageFilter) {
    this.messageFilter = messageFilter;
  }

  public String apply() {
    if(selected != null) {
      selected.clear();
    }
    init = true;
    loadingIsNeeded();
    return null;
  }

  public List<String> getSelected() {
    return selected;
  }

  public void setSelected(List<String> selected) {
    this.selected = selected;
  }

  public boolean isInit() {
    return init;
  }

  public void setInit(boolean init) {
    this.init = init;
  }

  public String clear() {
    if(selected != null) {
      selected.clear();
    }
    init = true;
    messageFilter = new MessageFilter();
    loadingIsNeeded();
    return null;
  }

  public String removeSelected() {
    if(selected != null && !selected.isEmpty()) {
      try{
        CCSms[] toCancel = new CCSms[selected.size()];
        int i = 0;
        for(String s : selected) {
          StringTokenizer t = new StringTokenizer(s, "_");
          CCSms sms = new CCSms(t.nextToken(), new Address(t.nextToken()), new Address(t.nextToken()));
          toCancel[i++] = sms;
        }
        wcontext.getOperativeStoreManager().cancelSMS(toCancel);
        selected.clear();
        loadingIsNeeded();
      }catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public List<SelectItem> getSmscInstances() {
    List<SelectItem> res = new ArrayList<SelectItem>(smscInstancesNumber+1);
    res.add(new SelectItem(-1, getLocalizedString("smsc.sms.view.smsc.instance.all")));
    for(int i=0; i < smscInstancesNumber; i++) {
      res.add(new SelectItem(i, Integer.toString(i+1)));
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


  private static Comparator<Object> getComparator(final DataTableSortOrder sortOrder) {
    return new Comparator<Object>() {
      public int compare(Object obj1, Object obj2) {
        Sms o1 = (Sms)obj1;
        Sms o2 = (Sms)obj2;
        int preres = sortOrder == null || sortOrder.isAsc() ? 1 : -1;
        try{
          if(sortOrder == null || sortOrder.getColumnId().equals("id")) {
            preres *= o1.getId() < o2.getId() ? -1 : o1.getId() == o2.getId() ? 0 : 1;
          }else if(sortOrder.getColumnId().equals("sendDate")) {
            if(o1.getSubmitTime() == null) {
              if(o2.getSubmitTime() == null) {
                return 0;
              }else {
                return -1*preres;
              }
            }else {
              if(o1.getSubmitTime() == null) {
                return preres;
              }
            }
            preres *= o1.getSubmitTime().compareTo(o2.getSubmitTime());
          }else if(sortOrder.getColumnId().equals("lastDate")) {
            if(o1.getLastTryTime() == null) {
              if(o2.getLastTryTime() == null) {
                return 0;
              }else {
                return -1*preres;
              }
            }else {
              if(o1.getLastTryTime() == null) {
                return preres;
              }
            }
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
        for(Collection<Sms> c : msgs) {
          for (Sms row : c) {
            appendRow(dateFormatter, writer, row);
          }
        }
      }else {
        for (Sms row : (Collection<Sms>)msgs[currentInstance]) {
          appendRow(dateFormatter, writer, row);
        }
      }
    }catch (AdminException e) {
      addError(e);
    }
  }


  protected LoadListener load(final Locale locale) {
    LoadListener listener = null;
    if(!loaded) {
      if(loadListener == null) {
        loadListener = new LoadListener();
        new Thread() {
          public void run() {
            try{
              final int smscInstancesNumber = getSmscInstancesNumber();
              loadListener.setTotal(smscInstancesNumber * 100);

              msgs = new Collection[smscInstancesNumber];

              totalSize = 0;
              ResourceBundle bundle = ResourceBundle.getBundle("ru.novosoft.smsc.web.resources.Smsc", locale);
              for (int i = 0; i < smscInstancesNumber; i++) {
                final int curI = i;
                msgs[i] = new LinkedList<Object>();
                for(Message m : wcontext.getOperativeStoreManager().getMessages(i, messageFilter, new ProgressObserver() {
                  public void update(long current, long total) {
                    int c = (int)(current);
                    int t = (int)(total);
                    loadListener.setCurrent((c * 100 / t / smscInstancesNumber) + (curI * 100 / smscInstancesNumber));
                    loadListener.setTotal(100);
                  }
                })) {
                  msgs[i].add(new OperativeSms(m, isAllowToShowSmsText(m.getSrcSmeId(), m.getDstSmeId()), bundle));
                }
                totalSize += msgs[i].size();
              }
              loaded = true;
            }catch (AdminException e){
              logger.error(e,e);
              loadListener.setLoadError(new ModelException(e.getMessage(locale)));
            }catch (Exception e){
              logger.error(e, e);
              loadListener.setLoadError(new ModelException(e.getLocalizedMessage() != null ? e.getLocalizedMessage() : e.getMessage() != null
                  ? e.getMessage() : "Internal error"));
            }
          }
        }.start();
      }
      listener = loadListener;
    }
    return listener;
  }

  public DataTableModel getMessages() {
    final Locale locale = getLocale();
    return new DataTableModelImpl(locale);
  }

  private class DataTableModelImpl implements PreloadableModel, ModelWithObjectIds {
    private final Locale locale;

    private DataTableModelImpl(Locale locale) {
      this.locale = locale;
    }

    public LoadListener prepareRows(int i, int i1, DataTableSortOrder dataTableSortOrder) throws ModelException {
      return load(locale);
    }
    public List getRows(int startPos, int count, DataTableSortOrder dataTableSortOrder) throws ModelException {
      final int _instance = currentInstance;
      if(msgs == null || _instance >= msgs.length || (_instance != -1 && msgs[_instance] == null) ) {
        return Collections.emptyList();
      }
      List<Object> result = new LinkedList<Object>();
      List<Object> records;
      if(_instance == -1) {
        records = new ArrayList<Object>(totalSize);
        for(Collection c : msgs)  {
          records.addAll(c);
        }
      }else {
        records = new ArrayList<Object>(msgs[_instance]);
      }
      Collections.sort(records, getComparator(dataTableSortOrder));
      for (int i = startPos; i < Math.min(records.size(), startPos + count); i++) {
        result.add(records.get(i));
      }
      return result;
    }
    public int getRowsCount() throws ModelException {
      final int _instance = currentInstance;
      if(msgs == null || _instance >= msgs.length || (_instance != -1 &&  msgs[_instance] == null)) {
        return 0;
      }
      return  _instance == -1 ? totalSize : msgs[_instance].size();
    }

    public String getId(Object o) throws ModelException {
      try{
        if(o instanceof Sms) {
          Sms sms = (Sms)o;
          return sms.getId()+"_"+sms.getOriginatingAddress().getSimpleAddress()+'_'+sms.getDestinationAddress().getSimpleAddress();
        }else {
          return null;
        }
      }catch (AdminException e) {
        addError(e);
        throw new ModelException(e.getMessage(locale));
      }
    }
  }

  public void viewSms(ActionEvent e) {
    String s_id = getRequestParameter("sms_id");
    long id = Long.parseLong(s_id);
    Sms sms = null;
    try{
      if(currentInstance == -1) {
        for(Collection c : msgs) {
          for(Sms s : (Collection<Sms>)c) {
            if(s.getId() == id) {
              sms = s;
              break;
            }
          }
          if(sms != null) {
            break;
          }
        }
      }else {
        for(Sms s : (Collection<Sms>)msgs[currentInstance]) {
          if(s.getId() == id) {
            sms = s;
            break;
          }
        }
      }
    }catch (AdminException ex) {
      addError(ex);
    }
    getRequest().put(SmsController.SMS_PARAM_NAME, sms);
  }
}
