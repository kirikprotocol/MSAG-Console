package ru.novosoft.smsc.infosme.backend.commands;

import ru.novosoft.smsc.admin.console.commands.infosme.InfoSmeDistr;
import ru.novosoft.smsc.admin.console.commands.infosme.Distribution;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.RandomAccessFileReader;
import ru.novosoft.smsc.util.config.Config;

import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import org.xml.sax.SAXException;
import org.apache.log4j.Category;

/**
 *
 * User: alkhal
 * Date: 22.10.2008
 *
 */

public class InfoSmeDistrImpl implements InfoSmeDistr {

  private static Category log = Category.getInstance(InfoSmeDistrImpl.class);

  public void resendMessage(CommandContext ctx, String msisdn, String taskId) {
    try {
      final SMSCAppContext appContext = ctx.getOwner().getContext();
      if (!appContext.getSmeManager().smeStatus("InfoSme").isConnected()) {
        ctx.setMessage("InfoSme is not started");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }
      final InfoSmeContext context = InfoSmeContext.getInstance(appContext, "InfoSme");
   //todo    context.getInfoSme().resendMessages(taskId, id, Message.State.NEW, new Date());
      Config config = new Config(new File(appContext.getHostsManager().getServiceInfo("InfoSme").getServiceFolder(),
        "conf" + File.separatorChar + "config.xml"));
      String storeDir = appContext.getHostsManager().getServiceInfo("InfoSme").getServiceFolder()
          +File.separator+config.getString("InfoSme.storeLocation");
      System.out.println("storeDir "+storeDir);
      long id = getId(msisdn, taskId, storeDir);
      if(id>0) {
        context.getInfoSme().resendMessages(taskId, Long.toString(id), Message.State.NEW, new Date());
        System.out.println("Line "+id+" found in Store for msisdn="+msisdn);
      } else {
        System.out.println("Line not found in Store for msisdn="+msisdn);
      }
      ctx.setMessage("OK");
      ctx.setStatus(CommandContext.CMD_OK);
    } catch (Exception e) {
      e.printStackTrace();
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }

  }

  public void createDistribution(CommandContext ctx, Distribution d) {
    try {
      try{
        validateNull(d.getDateBegin(),"dateBegin");
        validateNull(d.getDateEnd(),"dateEnd");
        validateNull(d.getTimeBegin(),"timeBegin");
        validateNull(d.getTimeEnd(),"timeEnd");
        validateNull(d.getDays(),"days");
        validateNull(d.isTxmode(),"txmode");
        validateNull(d.getAddress(),"address");
      } catch(Exception e) {
        ctx.setMessage("Wrong reguest command, some parameters type is unsupported: "+e.getMessage());
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }
      File f = new File(d.getFile());
      if (!f.exists()) {
        ctx.setMessage("File " + d.getFile() + " does not exist");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }


      final SMSCAppContext appContext = ctx.getOwner().getContext();
      if (!appContext.getSmeManager().smeStatus("InfoSme").isConnected()) {
        ctx.setMessage("InfoSme is not started");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }

      final InfoSmeContext context = InfoSmeContext.getInstance(appContext, "InfoSme");


      TaskBuilder taskBuilder = new TaskBuilder(context, d);
      String taskId;
      if((taskId = taskBuilder.getTaskId())==null) {
        throw new Exception("InfoSme Internal error");
      }
      taskBuilder.start();
      ctx.setMessage(taskId);
      ctx.setStatus(CommandContext.CMD_OK);
    } catch (Exception e) {
      e.printStackTrace();
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public void getStatus(CommandContext ctx, String taskId) {
    try{
      final SMSCAppContext appContext = ctx.getOwner().getContext();
      if (!appContext.getSmeManager().smeStatus("InfoSme").isConnected()) {
        ctx.setMessage("InfoSme is not started");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }

      final InfoSmeContext smeContext = InfoSmeContext.getInstance(appContext, "InfoSme");
      final String prefix = TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(taskId);
      String status;
      try{
        boolean loaded = smeContext.getConfig().getBool(prefix + ".messagesHaveLoaded");
        status = Boolean.toString(loaded);
      } catch(Exception e) {
        status="";
      }
      ctx.setMessage(status);
      ctx.setStatus(CommandContext.CMD_OK);

    } catch (Exception e) {
      e.printStackTrace();
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  private void validateNull(Object obj, String name) throws Exception{
    if((obj==null)||(obj.toString().equals(""))) {
      throw new Exception("Param empty: "+name);
    }
  }

  private long getId (String msisdn, String taskId, String storeDir) throws Exception{
    List files = getFiles(taskId, storeDir);
    Iterator iter = files.iterator();
    long id=-1;
    Calendar cal = Calendar.getInstance();
    long y1 = cal.get(Calendar.YEAR) % 10;
    long y2 = (cal.get(Calendar.YEAR) % 100) / 10;
    long m1 = (cal.get(Calendar.MONTH) + 1) % 10;
    long m2 = (cal.get(Calendar.MONTH) + 1) / 10;
    long d1 = cal.get(Calendar.DAY_OF_MONTH) % 10;
    long d2 = cal.get(Calendar.DAY_OF_MONTH) / 10;
    long h1 = cal.get(Calendar.HOUR_OF_DAY) % 10;
    long h2 = cal.get(Calendar.HOUR_OF_DAY) / 10;

    long idbase = y2 << 60 | y1 << 56 | m2 << 52 | m1 << 48 | d2 << 44 | d1 << 40 | h2 << 36 | h1 << 32;

    String msisdn2;
    if(msisdn.startsWith("+7")) {
      msisdn2="8"+msisdn.substring(2);
    } else {
      msisdn2 = "+7"+msisdn.substring(1);
    }
    while(iter.hasNext()) {
      File file = (File)iter.next();
      System.out.println("Analysis file:"+file.getAbsolutePath());
      RandomAccessFile f = null;
      int j=0;
      if (log.isDebugEnabled())
        log.debug("Start reading messages from file: " + file.getName());
      try {
        f = new RandomAccessFile(file, "r");

        RandomAccessFileReader is = new RandomAccessFileReader(f);
        String encoding = System.getProperty("file.encoding");
        String line = is.readLine(encoding); // Skip first string

        while(true) {
          long offset =is.getFilePointer();
          line = is.readLine(encoding);
          if (line == null)
            break;

          j++;

          StringTokenizer st = new StringTokenizer(line, ",");
          int state = Integer.parseInt(st.nextToken().trim());
          if (state != Message.State.DELIVERED.getId())
            continue;

          st.nextToken();
          String ms = st.nextToken().trim();
          if((!ms.equals(msisdn))&&(!ms.equals(msisdn2))){
            continue;
          }
          id = idbase | ((int)offset);
          break;
        }
        if(id>0) {
          break;
        }
      } catch (Exception e) {
        throw e;
      }  finally {
        if (f != null)
          try {
            f.close();
          } catch (IOException e) {
          }
      }
      if (log.isDebugEnabled())
        log.debug(j + " messages have readed from file: " + file.getName());

    }
    return id;

  }

  private List getFiles (String taskId, String storeDir) throws Exception{
    SimpleDateFormat dirNameFormat = new SimpleDateFormat("yyMMdd");
    SimpleDateFormat fileNameFormat = new SimpleDateFormat("HH");
    Date till = new Date();
    List files = new LinkedList();

    File dir = new File(storeDir + File.separator + taskId);
    if(dir.exists()) {
      File[] dirArr = dir.listFiles();
      for (int i=0;i<dirArr.length;i++) {
        File directory = dirArr[i];
        if(directory.isDirectory()) {
          String dirName = directory.getName();
          Date dirDate = dirNameFormat.parse(dirName);
          File[] fileArr = directory.listFiles();
          for(int j=0;j<fileArr.length;j++) {
            File f = fileArr[j];
            if (!f.isFile()) {
              continue;
            }
            String name = f.getName();
            if (name.lastIndexOf(".csv") < 0) {
              continue;
            }
            Date fileDate = fileNameFormat.parse(name.substring(0, name.lastIndexOf(".")));
            Date date = new Date(dirDate.getTime()+fileDate.getTime()+7*60*60*1000);
            if (date.compareTo(till) <= 0) {
              files.add(f);
            }
          }
        }
      }
    }
    return files;
  }
}
