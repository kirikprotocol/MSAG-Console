package ru.novosoft.smsc.admin.smsview;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.smsview.operative.Message;
//import ru.novosoft.smsc.admin.smsview.archive.Message;
import ru.novosoft.smsc.admin.smsview.operative.RsFileMessage;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCJspException;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.util.config.Config;

import java.io.*;
import java.util.*;


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
    String dirPrefics="/"; //Solaris
    int len = configPath.lastIndexOf(dirPrefics) + 1; //Solaris
    if (len <1) { dirPrefics="\\";//Windows
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
    BufferedInputStream input = null;
    BufferedInputStream input2 = null;
    //RandomAccessFile input=null;
    SmsSet set = new SmsSet();
    set.setHasMore(false);
    Map smes = new HashMap();
    Map smesId = new HashMap();
    Set smesPoint = new HashSet();
    int rowsMaximum = query.getRowsMaximum();
    int bufferSize=2048;
    if (rowsMaximum == 0) return set;
    boolean allReaded = false;
    try {
      input = new BufferedInputStream(new FileInputStream(smsstorePath),bufferSize);
     // input = new FileInputStream(smsstorePath);
      //input=new RandomAccessFile(smsstorePath,"r");
    } catch (FileNotFoundException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.

    }
    // QueryMessage request = new QueryMessage(query);
    //Message responce=new RsFileMessage(new SmsRow(),new SmsId()); //inicialising static field

    Message responce;
    System.out.println("start reading File in: "+new Date());
    try {
      String FileName = RsFileMessage.readString(input, 9);
      int version = (int) RsFileMessage.readUInt32(input);
      int toReceive = (rowsMaximum < MAX_SMS_FETCH_SIZE) ? rowsMaximum : MAX_SMS_FETCH_SIZE;
      long offset = 13;
      while (!allReaded) {
        responce = new RsFileMessage();
        try {
          responce.receiveSms(input );

        } catch (EOFException e) {
         System.out.println("EOFException");
          ((RsFileMessage)responce).setAllReaded(true);//break;
        }
        allReaded = ((RsFileMessage)responce).isAllReaded();
        SmsRow smsNew = ((RsFileMessage)responce).getSms();
        if (!querySelect(query, smsNew)) continue;
        if ( responce== null)
          throw new AdminException("Message from file  is NULL");
        if (!allReaded || ((RsFileMessage)responce).isBodyRecived()) {
          Long msgId = new Long(smsNew.getId());
          SmsId smsOld = (SmsId) smesId.get(msgId);
          //Long counter=(Long)smesId.get(msgId);
          if (smsOld != null) {smes.remove(msgId); set.setHasMore(false);}
           if (!set.isHasMore()) {
            smes.put(msgId, smsNew);
          }
          if (--toReceive <= 0) {
            toReceive = rowsMaximum - smes.size();
            if (toReceive <= 0) {
              set.setHasMore(true);
             // break;
            }
            else {
              toReceive = (toReceive < MAX_SMS_FETCH_SIZE) ? toReceive : MAX_SMS_FETCH_SIZE;
            }
          }// if
        } //if (!allReaded || ((RsFileMessage) responce).isBodyRecived())
     }// while (!allReaded)
      System.out.println("end reading smsId in: "+new Date());
    //  smesPoint.addAll(smesId.values());
      System.out.println("end putting smesPoint in: "+new Date());
     // allReaded = false;  RsFileMessage.setAllReaded(false);//for static sluchay
   /*   System.out.println("end reading smsId in: "+new Date());
      System.out.println("opening second file: ");
      input2 = new BufferedInputStream(new FileInputStream(smsstorePath));
      System.out.println("opened second file sucessfully: ");
      String  FileName2 = RsFileMessage.readString(input2, 9);
      int version2 = (int) RsFileMessage.readUInt32(input2);

       i=1; boolean skipSms=false;
      while (!allReaded) {
       // Message message = new RsFileMessage();
           try {
             if (smesPoint.contains(new Long(i))) {RsFileMessage.receiveSms(input2); skipSms=false;}
             else {RsFileMessage.skipSms(input2 ); skipSms=true;}
           } catch (EOFException e) {
              System.out.println("EOFException");   //To change body of catch statement use File | Settings | File Templates.
             RsFileMessage.setAllReaded(true);//break;
           }
        //responce=receiveSms(input2 );
       i++; allReaded = RsFileMessage.isAllReaded();
       if (skipSms) continue;
        SmsRow smsNew = RsFileMessage.getSms();
        if (!querySelect(query, smsNew)) continue;
        if (offset == 13)
          throw new AdminException("Message from file  is NULL");
        if (!allReaded || RsFileMessage.isBodyRecived()) {
          Long msgId = new Long(smsNew.getId());
       //   SmsRow smsOld = (SmsRow) smes.get(msgId);
        /*  if (smsOld != null) {
            smes.remove(msgId);set.setHasMore(false);
          }
          if (!set.isHasMore()) {
            smes.put(msgId, smsNew);
          }
          if (--toReceive <= 0) {
            toReceive = rowsMaximum - smes.size();
            if (toReceive <= 0) {
              set.setHasMore(true);
             // break;
            }
            else {
              toReceive = (toReceive < MAX_SMS_FETCH_SIZE) ? toReceive : MAX_SMS_FETCH_SIZE;
            }
          }// if (--toReceive <= 0)
        } //if (!allReaded || ((RsFileMessage) responce).isBodyRecived())
     }// while (!allReaded)
      allReaded = false;  RsFileMessage.setAllReaded(false);//for static sluchay
      // input2 = new FileInputStream(smsstorePath);
 /*     for (Iterator iterator = smes.values().iterator(); iterator.hasNext();) {
       SmsRow sms =  (SmsRow)iterator.next();
       Long msgId = new Long(sms.getId());
       offset=sms.getPointer();
       input2.skip(offset);
       responce = receiveSms(input2);
       SmsRow smsNew = ((RsFileMessage) responce).getSms();
       //smes.remove(msgId);
       smes.put(msgId,smsNew);
     }   */
     System.out.println("end reading sms in: "+new Date());

    } catch (IOException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
    } finally {
      try {
        if (input != null) input.close();
        if (input2 != null) input2.close();
      } catch (Exception exc) {
      }
      ;

    }
    set.addAll(smes.values());
    System.out.println("end adding sms in set in: "+new Date());
    return set;
  }
  public boolean querySelectId(SmsQuery query, SmsId smsId)
  {
    boolean allowed = true;
    if (needExpression(query.getSmsId())) {
      if (!String.valueOf(smsId).equalsIgnoreCase(query.getSmsId())) allowed = false;
    }
   return allowed;
  }

  public boolean querySelect(SmsQuery query, SmsRow sms)
  {
    boolean allowed = true;
    //boolean allowedSmsId = true;
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
    boolean finalResult=true;
    final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";
   // SmsRow sms = ((RsFileMessage) responce).getSms();

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
    finalResult= allowed  && allowedAbAddress && allowedAddress && allowedRouteId && allowedSmeId &&
            allowedSrcSmeId && allowedDstSmeId && allowedFromTime && allowedTillTime && allowedStatus &&
            allowedLastResult;
    return finalResult;
  }
/*
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
 /*
  public Message receiveId(InputStream input, long offset) throws IOException
  {
    Message message = new RsFileMessage();
    try {
      message.receiveId(input, offset);
    } catch (EOFException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
      ((RsFileMessage) message).setAllReaded(true);//break;
    }
    return message;
  }*/
   public Message receiveSms(BufferedInputStream input ) throws IOException
  {
    Message message = new RsFileMessage();
    try {
      message.receiveSms(input);
    } catch (EOFException e) {
       System.out.println("EOFException");
      ((RsFileMessage) message).setAllReaded(true);//break;
    }
    return message;
  }
  /*
  public Message receiveSms(InputStream input , long pointer) throws IOException
  {
    Message message = new RsFileMessage();
    try {
      message.receiveSms(input,  pointer);
    } catch (EOFException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
      ((RsFileMessage) message).setAllReaded(true);//break;
    }
    return message;
  }
    */

  public SmsSet getSmsCount(SmsQuery query) throws AdminException
  {
   BufferedInputStream input = null;
    SmsSet set = new SmsSet();
    set.setHasMore(true);
    Map smes = new HashMap();
    int rowsMaximum = query.getRowsMaximum();
    if (rowsMaximum == 0) return set;
    int bufferSize=2048;
    boolean allReaded = false;
    int toReceive = (rowsMaximum < MAX_SMS_FETCH_SIZE) ? rowsMaximum : MAX_SMS_FETCH_SIZE;
    int smsCount = 0;
    try {
     // input = new FileInputStream(smsstorePath);
       input=new BufferedInputStream(new FileInputStream(smsstorePath),bufferSize);
      Message responce;
      try {
        String FileName = RsFileMessage.readString(input, 9);
        int version = (int) RsFileMessage.readUInt32(input);
        while (!allReaded) {
          responce = receiveSms(input);
          allReaded = ((RsFileMessage) responce).isAllReaded();
          SmsRow smsNew = ((RsFileMessage) responce).getSms();
          if (!querySelect(query, smsNew)) continue;
          if (responce == null)
            throw new AdminException("Message from file  is NULL");
          if (!allReaded || ((RsFileMessage) responce).isBodyRecived()) {

            Long msgId = new Long(smsNew.getId());
            SmsRow smsOld = (SmsRow) smes.get(msgId);
            if (smsOld != null) {smes.remove(msgId);  smsCount--;set.setHasMore(false);}
            if (!set.isHasMore()) {
              //set.addRow(smsNew);
              smes.put(msgId, smsNew);
            }
            smsCount++;
            toReceive = rowsMaximum - smes.size();//set.getRowsCount();
            if (toReceive <= 0) {
              set.setHasMore(true);
         }// if (--toReceive <= 0)
        } //if (!allReaded || ((RsFileMessage) responce).isBodyRecived())
     }// while (!allReaded)
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
       set.addAll(smes.values());
    }  set.setSmesRows(smsCount);
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
