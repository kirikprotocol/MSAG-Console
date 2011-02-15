package ru.novosoft.smsc.admin.smsview;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.smsview.archive.Message;
import ru.novosoft.smsc.admin.smsview.operative.LazySmsRow;
import ru.novosoft.smsc.admin.smsview.operative.RsFileMessage;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.Functions;
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
  Category logger = Category.getInstance(SmsOperativeSource.class);
//  private final static int MAX_SMS_FETCH_SIZE = 200;
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
    if( !((new File(smsstorePath).exists())) )
       throw new AdminException("Operative store path does not exists "+smsstorePath);
  }

  protected void load(File file) throws IOException
  {
    FileReader reader = new FileReader(file);
    reader.read();
  }

  public SmsSet getSmsSet(SmsQuery query) throws AdminException
  {
    return getRows(query, true);
  }


  private SmsSet getRows1(SmsQuery query, boolean calcExactCount) {
    InputStream input = null;
    SmsSet set = new SmsSet();
    set.setHasMore(true);
    int rowsMaximum = query.getRowsMaximum();
    if (rowsMaximum == 0) return set;
    boolean haveArc=false;
    HashMap msgs = new HashMap(5000);
    System.out.println("start reading File in: " + new Date());
    long tm = System.currentTimeMillis();
    try {
      input = new FileInputStream(smsstorePath);  //todo Buffered OutputStream

      Message.readString(input, 9);
      long version = Message.readUInt32(input);
      if ( version> 0x010000 ) haveArc=true;
//      HashSet msgIds = new HashSet(5000);
      int totalCount = 0;
      try {
        RsFileMessage resp = new RsFileMessage();
        byte message[] = new byte[256*1024];
        while(true) {
          int msgSize1 = (int) Message.readUInt32(input);
          Functions.readBuffer(input, message, msgSize1);
          int msgSize2 = (int) Message.readUInt32(input);
          if (msgSize1 != msgSize2) throw new IOException("Protocol error sz1=" + msgSize1 + " sz2=" + msgSize2);
          InputStream bis = new ByteArrayInputStream(message, 0, msgSize1); // todo do not use ArrayInputStream
          long msgId=Message.readInt64(bis);
          Long lmsgId = new Long(msgId);   // todo Create lmsgId in if statement near  -
          totalCount++;                                                         //      |
          if( resp.receive(bis, query, message, msgId, false, haveArc) ) {       //     |
                                                                                //    <-
            if( resp.getSms().getStatusInt() == SmsRow.MSG_STATE_ENROUTE ) {
              msgs.put(lmsgId, resp.getSms());
            } else {
              msgs.remove(lmsgId);   // todo do not read all message if status != ENROUTE
            }
          }
        }
      } catch (EOFException e) {
      }
      logger.info("Operative store read "+totalCount+" records. "+msgs.size()+" messages matches filter");
      if( query.isFilterLastResult || query.isFilterStatus ) {   // todo move this into previous circle ???
        Map.Entry entry = null;
        SmsRow row = null;
        for( Iterator it = msgs.entrySet().iterator(); it.hasNext(); ) {
          entry = (Map.Entry)it.next();
          row = (SmsRow) entry.getValue();
          if( query.isFilterStatus && row.getStatusInt() != query.getStatus() ) {
            it.remove();
            continue;
          }
          if( query.isFilterLastResult && row.getLastResult() != query.getLastResult() ) {
            it.remove();
          }
        }
        logger.info("Operative store "+totalCount+" records. LastResult and Status filters processed "+msgs.size()+" messages matches filter");
      }
    } catch (Exception e) {
      e.printStackTrace();
      logger.error("Unexpected exception occured reading operative store file", e);
    } finally {
      if( input != null )
        try {
          input.close();
        } catch (IOException e) {
          logger.warn("can't close file");
        }
      System.out.println("end reading File in: " + new Date()+" spent: "+(System.currentTimeMillis()-tm)/1000);
      set.addAll(msgs.values());  // todo DO NOT COPY. USE set from the beginning!!!
    }
    set.setSmesRows(msgs.size());
    return set;
  }

  private SmsSet getRows(SmsQuery query, boolean calcExactCount) {
    SmsSet set = new SmsSet();
    set.setHasMore(true);
    int rowsMaximum = query.getRowsMaximum();
    if (rowsMaximum == 0) return set;
    boolean haveArc=false;

    // Check .row file does not exists
    String rowFile = smsstorePath.substring(0, smsstorePath.lastIndexOf('.')) + ".row";
    while (new File(rowFile).exists()) {
      logger.error("It seems that SMSC is making operative storage rolling. Wait one second before next try...");
      try {
        Thread.sleep(1000);
      } catch (InterruptedException e) {
        e.printStackTrace();
        return set;
      }
    }

    HashMap msgs = new HashMap();
    HashSet finishedMsgs = new HashSet();
    HashSet totalCounter = new HashSet();

    int msgsSize;
    boolean outOfSize = false;

    do {
      msgsSize = msgs.size();
      
      long tm = System.currentTimeMillis();
      InputStream input = null;
      try {
        input = new BufferedInputStream(new FileInputStream(smsstorePath));

        Message.readString(input, 9);
        long version = Message.readUInt32(input);
        if ( version> 0x010000 ) haveArc=true;

        try {
          while(true) {
            int msgSize1 = (int) Message.readUInt32(input);
            long msgId = Message.readInt64(input);  // 8 bytes

            Long msgIdLong = new Long(msgId);
            boolean outOfSize1 = (msgs.size() == rowsMaximum && !msgs.containsKey(msgIdLong));
            outOfSize = outOfSize || outOfSize1;
            if ((query.isFilterSmsId && msgId != query.getSmsIdValue()) || finishedMsgs.contains(msgIdLong) || (!calcExactCount && outOfSize1)) {
              Message.skip(input, msgSize1 - 8 + 4);
              continue;
            }

            Message.skip(input, 4); // Skip seq (4 bytes)
            int fin = Message.readUInt8(input); // finall (1 byte)
            int status = Message.readUInt8(input); // 1 byte

            if (fin == 0) {// Not final
              byte[] message = new byte[msgSize1];
              Functions.readBuffer(input, message, msgSize1 - 8 - 5 - 1);
              int msgSize2 = (int) Message.readUInt32(input);
              if (msgSize1 != msgSize2) throw new IOException("Protocol error sz1=" + msgSize1 + " sz2=" + msgSize2);

              SmsRow sms = new LazySmsRow(message, haveArc, msgId, status);

              if( query.isFilterFromDate && sms.getSubmitTime().compareTo(query.getFromDate()) < 0) {continue;}
              if( query.isFilterTillDate && sms.getSubmitTime().compareTo(query.getTillDate()) >= 0) {continue;}
              if( query.isFilterFromAddress && !query.getFromAddressMask().addressConfirm(sms.getOriginatingAddressMask())) {continue;}
              if( query.isFilterToAddress && !query.getToAddressMask().addressConfirm(sms.getDealiasedDestinationAddressMask())) {continue;}
              if( query.isFilterAbonentAddress && !(query.getAbonentAddressMask().addressConfirm(sms.getOriginatingAddressMask()) || query.getAbonentAddressMask().addressConfirm(sms.getDealiasedDestinationAddressMask()))) { continue;}
              if( query.isFilterRouteId && !query.getRouteId().equals(sms.getRouteId()) ) {continue;}
              if( query.isFilterSrcSmeId && !query.getSrcSmeId().equals(sms.getSrcSmeId()) ) {continue;}
              if( query.isFilterDstSmeId && !query.getDstSmeId().equals(sms.getDstSmeId()) ) {continue;}
              if( query.isFilterSmeId && !(query.getSmeId().equals(sms.getSrcSmeId()) || query.getSmeId().equals(sms.getDstSmeId()))) { continue;}
              if( query.isFilterLastResult && sms.getLastResult() != query.getLastResult() ) { continue;}

              if (!outOfSize1)
                msgs.put(msgIdLong, sms);
              if (calcExactCount)
                totalCounter.add(msgIdLong);
            } else {// Final message
              msgs.remove(msgIdLong);
              finishedMsgs.add(msgIdLong);
              if (calcExactCount)
                totalCounter.remove(msgIdLong);
              Message.skip(input, msgSize1 - 8 - 5 - 1 + 4);
            }
          }
        } catch (EOFException e) {
        }

      } catch (Exception e) {
        logger.error("Unexpected exception occured reading operative store file", e);
      } finally {
        if( input != null )
          try {
            input.close();
          } catch (IOException e) {
            logger.warn("can't close file");
          }
        System.out.println("end reading File in: " + new Date()+" spent: "+(System.currentTimeMillis()-tm)/1000);
      }

      if (rowsMaximum == Integer.MAX_VALUE || (msgs.size() < rowsMaximum == !outOfSize))
        break;

    } while (msgsSize != msgs.size());

    set.addAll(msgs.values());
    set.setSmesRows(calcExactCount ? totalCounter.size() : msgs.size());
    return set;
  }

  public SmsSet getSmsCount(SmsQuery query) throws AdminException
  {
    return getRows(query, true);
  }


  public String getSmsstorePath()
  {
    return smsstorePath;
  }
}
