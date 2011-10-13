package ru.novosoft.smsc.web.controllers.sms_view;

import mobi.eyeline.util.jsf.components.data_table.model.*;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveMessageFilter;
import ru.novosoft.smsc.admin.archive_daemon.SmsRow;
import ru.novosoft.smsc.admin.archive_daemon.SmsSet;

import javax.faces.event.ActionEvent;
import javax.faces.model.SelectItem;
import java.io.IOException;
import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * author: Aleksandr Khalitov
 */
public class SmsViewArchiveController extends SmsViewController{

  private ArchiveMessageFilter smsFilter = new ArchiveMessageFilter();

  private ArchiveMessageFilter oldFilter;

  private List<Sms> msgs;

  private LoadListener loadListener;

  private boolean loaded;

  private boolean init = false;

  public SmsViewArchiveController() {
    oldFilter = new ArchiveMessageFilter(smsFilter);
  }

  public ArchiveMessageFilter getSmsFilter() {
    return smsFilter;
  }

  public List<SelectItem> getStatuses() {
    SmsRow.Status[] statuses = SmsRow.Status.values();
    List<SelectItem> res = new ArrayList<SelectItem>(statuses.length);
    for(SmsRow.Status s : statuses) {
      res.add(new SelectItem(s.toString(), s.toString()));
    }
    return res;
  }

  public String getStatus() {
    return smsFilter.getStatus() == null ? null : smsFilter.getStatus().toString();
  }

  public void setStatus(String status) {
    smsFilter.setStatus(status != null && status.length() != 0 ? SmsRow.Status.valueOf(status) : null);
  }

  public String apply() {
    init = true;
    if(!oldFilter.equals(smsFilter)) {
      loadingIsNeeded();
      oldFilter = new ArchiveMessageFilter(smsFilter);
    }
    return null;
  }

  public String clear() {
    init = true;
    smsFilter = new ArchiveMessageFilter();
    loadingIsNeeded();
    return null;
  }

  public boolean isInit() {
    return init;
  }

  public boolean isLoaded() {
    return loaded;
  }

  protected void loadingIsNeeded() {
    loaded = false;
    loadListener = null;
  }

  public String getSmsId() {
    return smsFilter.getSmsId() == null ? null : smsFilter.getSmsId().toString();
  }

  public void setSmsId(String smsId) {
    smsFilter.setSmsId(smsId != null && (smsId = smsId.trim()).length() != 0 ? Long.parseLong(smsId) : null);
  }

  public String getLastResult() {
    return smsFilter.getLastResult() == null ? null : smsFilter.getLastResult().toString();
  }

  public void setLastResult(String lastResult) {
    smsFilter.setLastResult(lastResult == null || (lastResult = lastResult.trim()).length() == 0 ? null : Integer.parseInt(lastResult));
  }

  private static final String smppStatusPrefix = "smsc.errcode.";

  public List<SelectItem> getLastResults() {
    Locale locale = getLocale();
    ResourceBundle bundle = ResourceBundle.getBundle("ru.novosoft.smsc.web.resources.Smsc", locale);
    Enumeration<String> enumeration = bundle.getKeys();
    Map<Integer,String> preres = new TreeMap<Integer, String>();
    while (enumeration.hasMoreElements()) {
      String key = enumeration.nextElement();
      if(key.length() > smppStatusPrefix.length() && key.startsWith(smppStatusPrefix)) {
        try{
          Integer code = Integer.parseInt(key.substring(smppStatusPrefix.length()));
          preres.put(code, "(" + code + ") " + bundle.getString(key));
        }catch (NumberFormatException ignored){}
      }
    }

    List<SelectItem> res = new ArrayList<SelectItem>(preres.size());
    for(Map.Entry<Integer, String> e : preres.entrySet()) {
      res.add(new SelectItem(e.getKey().toString(), e.getValue()));
    }

    return res;
  }


  private static Comparator<Sms> getComparator(final DataTableSortOrder sortOrder) {
    return new Comparator<Sms>() {
      public int compare(Sms o1, Sms o2) {
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
          logger.error(e,e);
        }
        return preres;
      }
    };
  }



  @SuppressWarnings({"unchecked"})
  @Override
  protected void _download(PrintWriter writer) throws IOException {
    if(!isLoaded() || msgs == null) {
      return;
    }
    SimpleDateFormat dateFormatter = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss", getLocale());
    try{
      for (Sms row : msgs) {
        appendRow(dateFormatter, writer, row);
      }
    }catch (AdminException e) {
      addError(e);
    }
  }


  public DataTableModel getMessages() {
    if(wcontext.getArchiveDaemonManager() == null) {
      return new EmptyDataTableModel();
    }
    final Locale locale = getLocale();
    return new DataTableModelImpl(locale);
  }

  private class DataTableModelImpl implements PreloadableModel {
    private final Locale locale;

    private DataTableModelImpl(Locale locale) {
      this.locale = locale;
    }

    public LoadListener prepareRows(int i, int i1, DataTableSortOrder dataTableSortOrder) throws ModelException {
      LoadListener listener = null;
      if(!loaded) {
        if(loadListener == null) {
          loadListener = new LoadListener();
          new Thread() {
            public void run() {
              try{
                loadListener.setCurrent(0);
                loadListener.setTotal(1);
                final SmsSet messages = wcontext.getArchiveDaemon().getSmsSet(smsFilter);
                List<SmsRow> rs =  messages.getRowsList();
                msgs = new ArrayList<Sms>(rs.size());
                for(SmsRow r : rs) {
                  msgs.add(new ArchiveSms(r, isAllowToShowSmsText(r.getSrcSmeId(), r.getDstSmeId())));
                }
                loadListener.setCurrent(1);
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
    public List getRows(int startPos, int count, DataTableSortOrder dataTableSortOrder) throws ModelException {
      if(msgs == null) {
        return Collections.emptyList();
      }
      List<Object> result = new LinkedList<Object>();
      Collections.sort(msgs, getComparator(dataTableSortOrder));
      for (int i = startPos; i < Math.min(msgs.size(), startPos + count); i++) {
        result.add(msgs.get(i));
      }
      return result;
    }
    public int getRowsCount() throws ModelException {
      return msgs == null ? 0 : msgs.size();
    }

    public String getId(Object o) throws ModelException {
      try{
        return o instanceof Sms ? Long.toString(((Sms)o).getId()) : null;
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
        for(Sms s : msgs) {
          if(s.getId() == id) {
            sms = s;
            break;
          }
        }
    }catch (AdminException ex) {
      addError(ex);
    }
    getRequest().put(SmsController.SMS_PARAM_NAME, sms);
  }
}
