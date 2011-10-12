package ru.novosoft.smsc.web.controllers.sms_view;

import mobi.eyeline.util.jsf.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.util.jsf.components.data_table.model.LoadListener;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveMessageFilter;
import ru.novosoft.smsc.admin.archive_daemon.SmsRow;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.StringEncoderDecoder;

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

  private Collection<SmsRow> msgs;

  private int totalSize;


  public SmsViewArchiveController() {
    oldFilter = new ArchiveMessageFilter(smsFilter);
  }

  @Override
  protected void _load(Locale locale, LoadListener loadListener) throws AdminException {
    //To change body of implemented methods use File | Settings | File Templates.
  }

  public ArchiveMessageFilter getSmsFilter() {
    return smsFilter;
  }

  public String apply() {
    if(!oldFilter.equals(smsFilter)) {
      loadingIsNeeded();
      oldFilter = new ArchiveMessageFilter(smsFilter);
    }
    return null;
  }

  public String clear() {
    smsFilter = new ArchiveMessageFilter();
    loadingIsNeeded();
    return null;
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

  @Override
  protected List getRows(int startPos, int count, DataTableSortOrder dataTableSortOrder) {
    return null;  //To change body of implemented methods use File | Settings | File Templates.
  }

  @Override
  protected int getRowsCount() {
    return 0;  //To change body of implemented methods use File | Settings | File Templates.
  }

  private static Comparator<SmsRow> getComparator(final DataTableSortOrder sortOrder) {
    return new Comparator<SmsRow>() {
      public int compare(SmsRow o1, SmsRow o2) {
        int preres = sortOrder == null || sortOrder.isAsc() ? 1 : -1;
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
        for (SmsRow row : msgs) {
          appendRow(dateFormatter, writer, row);
        }
    }catch (AdminException e) {
      addError(e);
    }
  }

  private static final char TAB = '\t';
  private static final String LINE_SEP = "\r\n";
  private static final String SPACE = " ";

  private void appendRow(SimpleDateFormat dateFormatter, PrintWriter writer, SmsRow row) throws AdminException {
    String id = Long.toString(row.getId());
    String submit = row.getSubmitTime() != null ? StringEncoderDecoder.encode(dateFormatter.format(row.getSubmitTime())) : SPACE;
    String valid = row.getValidTime() != null ? StringEncoderDecoder.encode(dateFormatter.format(row.getValidTime())) : SPACE;
    String last = row.getLastTryTime() != null ? StringEncoderDecoder.encode(dateFormatter.format(row.getLastTryTime())) : SPACE;
    String next = row.getNextTryTime() != null ? StringEncoderDecoder.encode(dateFormatter.format(row.getNextTryTime())) : SPACE;
    Address source = row.getOriginatingAddress();
    Address dest = row.getDestinationAddress();
    String route = row.getRouteId();
//    Message.Status status = row.getStatus();
    //id + "\t" + submit + "\t" + valid + "\t" + last + "\t" + next + "\t" + source + "\t" + dest + "\t" + route + "\t" + status + "\t";
//    writer.append(id).append(TAB).append(submit).append(TAB).append(valid).append(TAB).append(last).append(TAB).append(next).append(TAB);
//    writer.append(source.getSimpleAddress()).append(TAB).append(dest.getSimpleAddress()).append(TAB).append(route).append(TAB).append(status.toString()).append(TAB);
//    if (row.getOriginalText() != null && isAllowToShowSmsText(row)) {
//      writer.append(row.getOriginalText());
//    }
    writer.append(LINE_SEP);
  }

  private static final String MAP_PROXY = "MAP_PROXY";
  private static final String SPECIAL_ROLE1 = "smsView_smstext_p2p";
  private static final String SPECIAL_ROLE2 = "smsView_smstext_content";

  protected boolean isAllowToShowSmsText(SmsRow row) throws AdminException{
    if (row.getSrcSmeId().equalsIgnoreCase(MAP_PROXY) && row.getDstSmeId().equals(MAP_PROXY)) {
      return isUserhasRole(SPECIAL_ROLE1);
    }else {
      return isUserhasRole(SPECIAL_ROLE2);
    }
  }
}
