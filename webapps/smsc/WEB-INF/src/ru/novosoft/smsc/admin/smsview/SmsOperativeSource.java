package ru.novosoft.smsc.admin.smsview;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.smsview.archive.Message;
import ru.novosoft.smsc.admin.smsview.operative.RsFileMessage;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCJspException;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.util.config.Config;

import java.io.*;
import java.util.HashMap;
import java.util.Map;
import java.util.Date;
import java.util.HashSet;

import org.apache.log4j.Category;


/**
 * Created by IntelliJ IDEA.
 * User: andrey
 * Date: 10.02.2005
 * Time: 18:24:47
 * To change this template use File | Settings | File Templates.
 */
public class SmsOperativeSource extends SmsSource
{
  Category logger = Category.getInstance(SmsOperativeSource.class);
  private final static int MAX_SMS_FETCH_SIZE = 200;
  private String smsstorePath;
  private static final String SECTION_NAME_LocalStore = "MessageStore.LocalStore";
  private static final String PARAM_NAME_filename = "filename";
  //protected boolean allReaded=false;
  // ArchiveDaemonContext context = null;

  public void init(SMSCAppContext appContext) throws AdminException
  {
    //  context = ArchiveDaemonContext.getInstance(appContext);
    Smsc smsc = appContext.getSmsc();
    String configPath = smsc.getConfigFolder().getAbsolutePath();
    String dirPrefics = "/"; //Solaris
    int len = configPath.lastIndexOf(dirPrefics) + 1; //Solaris
    if (len < 1) {
      dirPrefics = "\\";//Windows
      len = configPath.lastIndexOf(dirPrefics) + 1;//Windows
    }
    String absolutePath = configPath.substring(0, len);
    Config config = smsc.getSmscConfig();

    try {
      smsstorePath = config.getString(SECTION_NAME_LocalStore + '.' + PARAM_NAME_filename);
    } catch (Config.ParamNotFoundException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
    } catch (Config.WrongParamTypeException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
    }
    if (smsstorePath.indexOf(dirPrefics) != 0)
      smsstorePath = absolutePath + smsstorePath;
    FileInputStream input = null;
    try {
      input = new FileInputStream(smsstorePath);
    } catch (FileNotFoundException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
      throw new AdminException(e.getMessage());
    }
  }

  protected void load(File file) throws IOException
  {
    FileReader reader = new FileReader(file);
    reader.read();
  }

  public SmsSet getSmsSet(SmsQuery query) throws AdminException
  {
    return getSmsCount(query);
  }

/*  public SmsSet getSmsSet(SmsQuery query) throws AdminException
  {
    FileInputStream input = null;
    SmsSet set = new SmsSet();
    set.setHasMore(false);
    Map smes = new HashMap();
    int rowsMaximum = query.getRowsMaximum();
    if (rowsMaximum == 0) return set;
    boolean allReaded = false;
    try {
      input = new FileInputStream(smsstorePath);
    } catch (FileNotFoundException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.

    }
    // QueryMessage request = new QueryMessage(query);
    Message responce;
    ;
    System.out.println("start reading File in: " + new Date());
    try {
      String FileName = Message.readString(input, 9);
      int version = (int) Message.readUInt32(input);
      int toReceive = (rowsMaximum < MAX_SMS_FETCH_SIZE) ? rowsMaximum : MAX_SMS_FETCH_SIZE;
      int counter = 0;
      while (!allReaded) {
        responce = receive(input);
        allReaded = ((RsFileMessage) responce).isAllReaded();
        if (!querySelect(query, responce)) continue;
        if (responce == null)
          throw new AdminException("Message from file  is NULL");
        if (!allReaded || ((RsFileMessage) responce).isBodyRecived()) {
          SmsRow smsNew = ((RsFileMessage) responce).getSms();
          Long msgId = new Long(smsNew.getId());
          SmsRow smsOld = (SmsRow) smes.get(msgId);
          if (smsOld == null) {
//            set.setHasMore(false);
            counter++;
          }
          if (!set.isHasMore()) {
            //set.addRow(smsNew);
            smes.put(msgId, smsNew);
          }
          else {
            smes.put(msgId, null);
          }
          if (--toReceive <= 0) {
            toReceive = rowsMaximum - smes.size();
            if (toReceive <= 0) {
              set.setHasMore(true);
            }
            else {
              toReceive = (toReceive < MAX_SMS_FETCH_SIZE) ? toReceive : MAX_SMS_FETCH_SIZE;
            }
          }

        }

      }
    } catch (IOException e) {
      logger.error("IO error", e);
    } finally {
      try {
        if (input != null) input.close();
      } catch (Exception exc) {
      }
      ;
    }
    System.out.println("end reading File in: " + new Date());
    set.addAll(smes.values());
    return set;
  }
*/
  public boolean querySelect(SmsQuery query, SmsRow sms)
  {
   /* boolean allowed = true;
    boolean allowedSmsId = true;
    boolean allowedAbAddress = true;
   */
    boolean allowedOrAddress = true;
    boolean allowedDDAddress = true;
   /* boolean allowedAddress = true;
    boolean allowedRouteId = true;
    boolean allowedSmeId = true;
    boolean allowedSrcSmeId = true;
    boolean allowedDstSmeId = true;
    boolean allowedFromTime = true;
    boolean allowedTillTime = true;
    boolean allowedStatus = true;
    boolean allowedLastResult = true;
    boolean finalResult = true;
   */
    final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";
    //SmsRow sms = ((RsFileMessage) responce).getSms();

    if (needExpression(query.getSmsId())) {
      if (!String.valueOf(sms.getId()).equalsIgnoreCase(query.getSmsId())) return false;
    }

    if (query.getFromDateEnabled()) { //list.add("SUBMIT_TIME >=?");
      if (sms.getSubmitTime().compareTo(query.getFromDate()) <= 0) return false;
    }
    if (query.getTillDateEnabled()) {//list.add("SUBMIT_TIME <=?");
      if (sms.getSubmitTime().compareTo(query.getTillDate()) >= 0) return false;
    }

    String abAddress = query.getAbonentAddress();
    String smsOrAdd = sms.getOriginatingAddress();
    String smsDDAdd = sms.getDealiasedDestinationAddress();
    if (needExpression(abAddress)) {
      if (needLikeExpression(abAddress)) { //  abonentCheck = " LIKE ?";
        if (smsOrAdd.indexOf(abAddress) == -1 && smsDDAdd.indexOf(abAddress) == -1) return false;
      }
      else { //  abonentCheck = "=?";
        if (!smsOrAdd.equalsIgnoreCase(abAddress) && !smsDDAdd.equalsIgnoreCase(abAddress)) return false;
      }
    }
    else {
      if (needExpression(query.getFromAddress()) && needSmsExpression(smsOrAdd)) {
        if (!smsOrAdd.equalsIgnoreCase(query.getFromAddress())) allowedOrAddress = false;
      }
      if (needExpression(query.getToAddress()) && needSmsExpression(smsDDAdd)) {
        if (!smsDDAdd.equalsIgnoreCase(query.getToAddress())) allowedDDAddress = false;
      }
      if (!allowedOrAddress || !allowedDDAddress) return false;
    }
    String RouteId = query.getRouteId();
    String smsRouteId = sms.getRouteId();
    String querySrcSmeId = query.getSrcSmeId();
    String smsSrcSmeId = sms.getSrcSmeId();
    if (needExpression(RouteId) && needSmsExpression(smsRouteId)) {
      if (!smsRouteId.equalsIgnoreCase(RouteId)) return false;
    }
    String SmeId = query.getSmeId();
    if (needExpression(SmeId)) {
      if (needSmsExpression(sms.getSrcSmeId()) && needSmsExpression(sms.getDstSmeId())) {
        if ((needLikeExpression(SmeId))) {//  smeCheck = " LIKE ?";
          if (sms.getSrcSmeId().indexOf(SmeId) == -1 && sms.getDstSmeId().indexOf(SmeId) == -1) return false;
        }
        else { //  smeCheck = "=?";
          if (!sms.getSrcSmeId().equalsIgnoreCase(SmeId) && !sms.getDstSmeId().equalsIgnoreCase(SmeId)) return false;
        }
      }
    }
    else {
      if (needExpression(query.getSrcSmeId()) && needSmsExpression(sms.getSrcSmeId())) {
        if (!sms.getSrcSmeId().equalsIgnoreCase(query.getSrcSmeId())) return false;
      }
      if (needExpression(query.getDstSmeId()) && needSmsExpression(sms.getDstSmeId())) {
        if (!sms.getDstSmeId().equalsIgnoreCase(query.getDstSmeId())) return false;
      }
    }


    if (query.getStatus() != SmsQuery.SMS_UNDEFINED_VALUE) {//list.add("ST=" + query.getStatus());
      if (!sms.getStatus().equalsIgnoreCase(String.valueOf(query.getStatus()))) return false;
    }
    if (query.getLastResult() != SmsQuery.SMS_UNDEFINED_VALUE) {// list.add("LAST_RESULT=" + query.getLastResult());
      if (sms.getLastResult() != query.getLastResult()) return false;
    }
    return true;
  }


  public Message receive(InputStream input) throws IOException
  {
    Message message = new RsFileMessage();
    try {
      message.receive(input);
    } catch (EOFException e) {
      ((RsFileMessage) message).setAllReaded(true);//break;
    }
    return message;
  }

  public SmsSet getSmsCount(SmsQuery query) throws AdminException
  {
    InputStream input = null;
    SmsSet set = new SmsSet();
    set.setHasMore(true);
    HashMap msgs = new HashMap(5000);
    HashSet msgIds = new HashSet(5000);
    int rowsMaximum = query.getRowsMaximum();
    if (rowsMaximum == 0) return set;
    boolean allReaded = false;
   // int toReceive = (rowsMaximum < MAX_SMS_FETCH_SIZE) ? rowsMaximum : MAX_SMS_FETCH_SIZE;
    int smsCount = 0;
    long tm = System.currentTimeMillis();
    try {
      input = new FileInputStream(smsstorePath);
      System.out.println("start reading File in: " + new Date());
      Message responce;
      try {
        String FileName = Message.readString(input, 9);
        int version = (int) Message.readUInt32(input);
        while (!allReaded) {
          responce = receive(input);
          allReaded = ((RsFileMessage) responce).isAllReaded();
          if (!allReaded ) {
          SmsRow smsNew = ((RsFileMessage) responce).getSms();
          if (!querySelect(query, smsNew)) continue;
            Long msgId = new Long(smsNew.getId());
            if ( msgIds.contains(msgId) ) {
              msgs.put(msgId, smsNew);
            } else {
              msgIds.add(msgId);
              if (msgs.size()<rowsMaximum) {
                msgs.put(msgId, smsNew);
              }
              smsCount++;
            }
          }// if (!allReaded )
        }// while (!allReaded)
      } catch (IOException e) {
        e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
      } finally {
        try {
          if (input != null) input.close();
        } catch (Exception exc) {
        }
      }
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try {
        if (input != null) input.close();
        input = null;
      } catch (Exception exc) {
      }
      System.out.println("end reading File in: " + new Date()+" spent: "+(System.currentTimeMillis()-tm)/1000);
      set.addAll(msgs.values());
    }
    set.setSmesRows(smsCount);
    return set;
  }

  private boolean needExpression(String str)
  {
    return (str != null && str.length() != 0 && !str.trim().equalsIgnoreCase("*"));
  }

  private boolean needSmsExpression(String str)
  {
    return (str != null && str.length() != 0);
  }

  private boolean needLikeExpression(String str)
  {
    return (str.indexOf('*') >= 0 || str.indexOf('?') >= 0);
  }

  public String getSmsstorePath()
  {
    return smsstorePath;
  }
}
