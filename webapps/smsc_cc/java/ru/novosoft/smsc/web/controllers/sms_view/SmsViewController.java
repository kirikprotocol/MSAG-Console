package ru.novosoft.smsc.web.controllers.sms_view;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SmscController;

import javax.faces.model.SelectItem;
import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * author: Aleksandr Khalitov
 */
public class SmsViewController extends SmscController{

  protected static final Logger logger = Logger.getLogger(SmsViewController.class);

  protected WebContext wcontext = WebContext.getInstance();

  protected static final char TAB = '\t';
  protected static final String LINE_SEP = "\r\n";
  protected static final String SPACE = " ";

  private boolean hasSpecialRole1 = false;
  private boolean hasSpecialRole2 = false;

  public SmsViewController() {
    hasSpecialRole1 = isUserhasRole(SPECIAL_ROLE1);
    hasSpecialRole2 = isUserhasRole(SPECIAL_ROLE2);
  }

  public List<SelectItem> getMaxRows() {
    List<SelectItem> res = new ArrayList<SelectItem>();
    Collections.addAll(res,
        new SelectItem(10),
        new SelectItem(100),
        new SelectItem(200),
        new SelectItem(300),
        new SelectItem(400),
        new SelectItem(500),
        new SelectItem(1000),
        new SelectItem(2000),
        new SelectItem(5000)
    );
    return res;
  }

  private static final String smppStatusPrefix = "smsc.errcode.";

  public List<SelectItem> getRoutes() {
    List<SelectItem> res = new LinkedList<SelectItem>();
    try{
      for(Route r : wcontext.getRouteSubjectManager().getSettings().getRoutes()) {
        res.add(new SelectItem(r.getName(), r.getName()));
      }
    }catch (AdminException e) {
      addError(e);
    }
    return res;
  }

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

  private static final String MAP_PROXY = "MAP_PROXY";
  private static final String SPECIAL_ROLE1 = "smsView_smstext_p2p";
  private static final String SPECIAL_ROLE2 = "smsView_smstext_content";


  protected boolean isAllowToShowSmsText(String srcSme, String dstSme) throws AdminException{
//    if (srcSme.equalsIgnoreCase(MAP_PROXY) && dstSme.equals(MAP_PROXY)) {
//      return hasSpecialRole1;
//    }else {
//      return hasSpecialRole2;
//    }
    return true; //todo!!!
  }

  protected void appendRow(SimpleDateFormat dateFormatter, PrintWriter writer, Sms row) throws AdminException {
    String id = Long.toString(row.getId());
    String submit = row.getSubmitTime() != null ? StringEncoderDecoder.encode(dateFormatter.format(row.getSubmitTime())) : SPACE;
    String valid = row.getValidTime() != null ? StringEncoderDecoder.encode(dateFormatter.format(row.getValidTime())) : SPACE;
    String last = row.getLastTryTime() != null ? StringEncoderDecoder.encode(dateFormatter.format(row.getLastTryTime())) : SPACE;
    String next = row.getNextTryTime() != null ? StringEncoderDecoder.encode(dateFormatter.format(row.getNextTryTime())) : SPACE;
    Address source = row.getOriginatingAddress();
    Address dest = row.getDestinationAddress();
    String route = row.getRouteId();
    SmsStatus status = row.getStatus();
    //id + "\t" + submit + "\t" + valid + "\t" + last + "\t" + next + "\t" + source + "\t" + dest + "\t" + route + "\t" + status + "\t";
    writer.append(id).append(TAB).append(submit).append(TAB).append(valid).append(TAB).append(last).append(TAB).append(next).append(TAB);
    writer.append(source.getSimpleAddress()).append(TAB).append(dest.getSimpleAddress()).append(TAB).append(route).append(TAB).append(status.toString()).append(TAB);
    if (row.getOriginalText() != null && isAllowToShowSmsText(row.getSrcSmeId(), row.getDstSmeId())) {
      writer.append(row.getOriginalText());
    }
    writer.append(LINE_SEP);
  }


}
