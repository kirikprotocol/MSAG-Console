package ru.novosoft.smsc.admin.smsview;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.smsview.archive.Message;
import ru.novosoft.smsc.admin.smsview.operative.RsFileMessage;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;

import java.io.*;

/**
 * Created by IntelliJ IDEA.
 * User: andrey
 * Date: 10.02.2005
 * Time: 18:24:47
 * To change this template use File | Settings | File Templates.
 */
public class SmsOperativeSource extends SmsSource
{
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
    int len = configPath.lastIndexOf("\\");
    String absolutePath = configPath.substring(0, len);
    Config config = smsc.getSmscConfig();
    String path="";
    try {
       smsstorePath = config.getString(SECTION_NAME_LocalStore + '.' + PARAM_NAME_filename);
    } catch (Config.ParamNotFoundException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
    } catch (Config.WrongParamTypeException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
    }
    if (smsstorePath.indexOf("/") == 0)
      path = absolutePath + smsstorePath;
     try {  FileInputStream input = null;
      input = new FileInputStream(path);
    } catch (FileNotFoundException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
    smsstorePath=absolutePath+"/"+smsstorePath;
    }
  }

  protected void load(File file) throws IOException
  {
    FileReader reader = new FileReader(file);
    reader.read();
  }

  public SmsSet getSmsSet(SmsQuery query) throws AdminException
  {
    FileInputStream input = null;
    SmsSet set = new SmsSet();
    set.setHasMore(false);
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

    try {
      String FileName = Message.readString(input, 9);
      int version = (int) Message.readUInt32(input);
      int toReceive = (rowsMaximum < MAX_SMS_FETCH_SIZE) ? rowsMaximum : MAX_SMS_FETCH_SIZE;
      while (!allReaded) {
        responce = receive(input);
        allReaded = ((RsFileMessage) responce).isAllReaded();
        if (!querySelect(query, responce)) continue;
        if (responce == null)
          throw new AdminException("Message from file  is NULL");
        if (!allReaded || ((RsFileMessage) responce).isBodyRecived()) {
          if (!set.isHasMore()) {
            SmsRow smsNew = ((RsFileMessage) responce).getSms();
            long msgId = smsNew.getId();
            SmsRow smsOld = set.getRow(msgId);
            if (smsOld != null) set.removeRow(smsOld);
            set.addRow(smsNew);
          }
          if (--toReceive <= 0) {
            toReceive = rowsMaximum - set.getRowsCount();
            if (toReceive <= 0) {
              set.setHasMore(true);
              break;
            }
            else {
              toReceive = (toReceive < MAX_SMS_FETCH_SIZE) ? toReceive : MAX_SMS_FETCH_SIZE;
            }
          }

        }

      }
    } catch (IOException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
    } finally {
      try {
        if (input != null) input.close();
      } catch (Exception exc) {
      }
      ;

    }

    return set;
  }

  public boolean querySelect(SmsQuery query, Message responce)
  {
    boolean allowed = true;
    boolean allowedSmsId = true;
    boolean allowedAbAddress = true;
    boolean allowedOrAddress = true;
    boolean allowedDDAddress = true;
    boolean allowedAddress = true;
    boolean allowedRouteId = true;
    boolean allowedSmeId = true;
    boolean allowedSrcSmeId = true;
    boolean allowedDstSmeId = true;
    boolean allowedFromTime = true;
    boolean allowedTillTime = true;
    boolean allowedStatus = true;
    boolean allowedLastResult = true;
    final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";
    SmsRow sms = ((RsFileMessage) responce).getSms();

    if (needExpression(query.getSmsId())) {
      if (!String.valueOf(sms.getId()).equalsIgnoreCase(query.getSmsId())) allowed = false;
    }
    String abAddress = query.getAbonentAddress();
    String smsOrAdd = sms.getOriginatingAddress();
    String smsDDAdd = sms.getDealiasedDestinationAddress();
    if (needExpression(abAddress)) {
      if (needLikeExpression(abAddress)) { //  abonentCheck = " LIKE ?";
        if (smsOrAdd.indexOf(abAddress) == -1 && smsDDAdd.indexOf(abAddress) == -1) allowedAbAddress = false;
      }
      else { //  abonentCheck = "=?";
        if (!smsOrAdd.equalsIgnoreCase(abAddress) && !smsDDAdd.equalsIgnoreCase(abAddress)) allowedAbAddress = false;
      }
    }
    else {
      if (needExpression(query.getFromAddress()) && needSmsExpression(smsOrAdd)) {
        if (!smsOrAdd.equalsIgnoreCase(query.getFromAddress())) allowedOrAddress = false;
      }
      if (needExpression(query.getToAddress()) && needSmsExpression(smsDDAdd)) {
        if (!smsDDAdd.equalsIgnoreCase(query.getToAddress())) allowedDDAddress = false;
      }
      if (!allowedOrAddress || !allowedDDAddress) allowedAddress = false;
    }
    String RouteId = query.getRouteId();
    String smsRouteId = sms.getRouteId();
    String querySrcSmeId = query.getSrcSmeId();
    String smsSrcSmeId = sms.getSrcSmeId();
    if (needExpression(RouteId) && needSmsExpression(smsRouteId)) {
      if (!smsRouteId.equalsIgnoreCase(RouteId)) allowedRouteId = false;
    }
    String SmeId = query.getSmeId();
    if (needExpression(SmeId)) {
      if (needSmsExpression(sms.getSrcSmeId()) && needSmsExpression(sms.getDstSmeId())) {
        if ((needLikeExpression(SmeId))) {//  smeCheck = " LIKE ?";
          if (sms.getSrcSmeId().indexOf(SmeId) == -1 && sms.getDstSmeId().indexOf(SmeId) == -1) allowedSmeId = false;
        }
        else { //  smeCheck = "=?";
          if (!sms.getSrcSmeId().equalsIgnoreCase(SmeId) && !sms.getDstSmeId().equalsIgnoreCase(SmeId)) allowedSmeId = false;
        }
      }
    }
    else {
      if (needExpression(query.getSrcSmeId()) && needSmsExpression(sms.getSrcSmeId())) {
        if (!sms.getSrcSmeId().equalsIgnoreCase(query.getSrcSmeId())) allowedSrcSmeId = false;
      }
      if (needExpression(query.getDstSmeId()) && needSmsExpression(sms.getDstSmeId())) {
        if (!sms.getDstSmeId().equalsIgnoreCase(query.getDstSmeId())) allowedDstSmeId = false;
      }
    }

    if (query.getFromDateEnabled()) { //list.add("SUBMIT_TIME >=?");
      if (sms.getSubmitTime().compareTo(query.getFromDate()) <= 0) allowedFromTime = false;
    }
    if (query.getTillDateEnabled()) {//list.add("SUBMIT_TIME <=?");
      if (sms.getSubmitTime().compareTo(query.getTillDate()) >= 0) allowedTillTime = false;
    }

    if (query.getStatus() != SmsQuery.SMS_UNDEFINED_VALUE) {//list.add("ST=" + query.getStatus());
      if (!sms.getStatus().equalsIgnoreCase(String.valueOf(query.getStatus()))) allowedStatus = false;
    }
    if (query.getLastResult() != SmsQuery.SMS_UNDEFINED_VALUE) {// list.add("LAST_RESULT=" + query.getLastResult());
      if (sms.getLastResult() != query.getLastResult()) allowedLastResult = false;
    }
    return allowed && allowedSmsId && allowedAbAddress && allowedAddress && allowedRouteId && allowedSmeId &&
            allowedSrcSmeId && allowedDstSmeId && allowedFromTime && allowedTillTime && allowedStatus &&
            allowedLastResult;
  }


  public Message receive(InputStream input) throws IOException
  {
    Message message = new RsFileMessage();
    try {
      message.receive(input);
    } catch (EOFException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
      ((RsFileMessage) message).setAllReaded(true);//break;
    }
    return message;
  }

  public int getSmsCount(SmsQuery query) throws AdminException
  {
    InputStream input = null;
    SmsSet set = new SmsSet();
    set.setHasMore(true);
    int rowsMaximum = query.getRowsMaximum();
    if (rowsMaximum == 0) return 0;
    boolean allReaded = false;
    int toReceive = (rowsMaximum < MAX_SMS_FETCH_SIZE) ? rowsMaximum : MAX_SMS_FETCH_SIZE;
    int smsCount = 0;
    try {
      input = new FileInputStream(smsstorePath);

      Message responce;
      try {
        String FileName = Message.readString(input, 9);
        int version = (int) Message.readUInt32(input);
        while (!allReaded) {
          responce = receive(input);
          allReaded = ((RsFileMessage) responce).isAllReaded();
          if (!querySelect(query, responce)) continue;
          if (responce == null)
            throw new AdminException("Message from file  is NULL");
          if (!allReaded || ((RsFileMessage) responce).isBodyRecived()) {
            if (!set.isHasMore()) {
              SmsRow smsNew = ((RsFileMessage) responce).getSms();
              long msgId = smsNew.getId();
              SmsRow smsOld = set.getRow(msgId);
              if (smsOld != null) set.removeRow(smsOld);
              set.addRow(smsNew);
            }
            smsCount++;
            toReceive = rowsMaximum - set.getRowsCount();
            if (toReceive <= 0) {
              set.setHasMore(true);
            }
          }

        }

      } catch (IOException e) {
        e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
      } finally {
        try {
          if (input != null) input.close();
        } catch (Exception exc) {
        }
        ;

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
      ;

    }
    return smsCount;
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
