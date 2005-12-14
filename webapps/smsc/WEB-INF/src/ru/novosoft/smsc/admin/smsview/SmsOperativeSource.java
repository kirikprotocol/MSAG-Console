package ru.novosoft.smsc.admin.smsview;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.smsview.archive.Message;
import ru.novosoft.smsc.admin.smsview.operative.RsFileMessage;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.config.Config;

import java.io.*;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;


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
    return getRows(query, true);
  }


  private SmsSet getRows(SmsQuery query, boolean calcExactCount) {
    InputStream input = null;
    SmsSet set = new SmsSet();
    set.setHasMore(true);
    int rowsMaximum = query.getRowsMaximum();
    if (rowsMaximum == 0) return set;
    boolean haveArc=false;
    int smsCount = 0;
    HashMap msgs = new HashMap(5000);
    System.out.println("start reading File in: " + new Date());
    long tm = System.currentTimeMillis();
    try {
      input = new FileInputStream(smsstorePath);

      String FileName = Message.readString(input, 9);
      long version = Message.readUInt32(input);
      if ( version> 0x010000 ) haveArc=true;
//      HashSet msgIds = new HashSet(5000);
      try {
        RsFileMessage resp = new RsFileMessage();
        byte message[] = new byte[256*1024];
        while(true) {
          int msgSize1 = (int) Message.readUInt32(input);
          Functions.readBuffer(input, message, msgSize1);
          int msgSize2 = (int) Message.readUInt32(input);
          if (msgSize1 != msgSize2) throw new IOException("Protocol error sz1=" + msgSize1 + " sz2=" + msgSize2);
          InputStream bis = new ByteArrayInputStream(message, 0, msgSize1);
          long msgId=Message.readInt64(bis);
          Long lmsgId = new Long(msgId);
          if( resp.receive(bis, query, message, msgId, false,haveArc) ) {
            if( resp.getSms().getStatusInt() == SmsRow.MSG_STATE_ENROUTE ) {
              msgs.put(lmsgId, resp.getSms());
            } else {
              msgs.remove(lmsgId);
            }
          }
        }
      } catch (EOFException e) {
        e.printStackTrace();
      }
      if( query.isFilterLastResult || query.isFilterStatus ) {
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
            continue;
          }
        }
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
      set.addAll(msgs.values());
    }
    set.setSmesRows(msgs.size());
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
