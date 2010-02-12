package ru.novosoft.smsc.infosme.backend.commands;

import org.apache.log4j.Category;

import java.util.*;
import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;

import ru.novosoft.smsc.infosme.backend.tables.messages.MessageDataSource;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageVisitor;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageDataItem;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageFilter;
import ru.novosoft.smsc.infosme.backend.Message;

/**
 * author: alkhal
 */
public class InfoSmeExportStatManager implements Runnable{

  private static final Category logger = Category.getInstance(InfoSmeExportStatManager.class);

  private final LinkedList queue = new LinkedList();

  private MessageDataSource ds;

  public InfoSmeExportStatManager(MessageDataSource ds) {
    this.ds = ds;
  }

  private boolean started = false;

  private final Object lock = new Object();

  private final Object shutdownLock = new Object();

  /*
   * returns -1 - if manager cat export stats
              0 - if task is not processed
              1 - if export is started
  */

  public int addExportTask(String taskId, String fileName, Date startDate) throws IOException{
    if(!started) {
      return -1;
    }
    if(taskId == null || fileName == null) {
      throw new IllegalArgumentException();
    }
    try{
      if(!ds.isAllMessagesProcessed(taskId)) {
        return 0;
      }
    }catch(ParseException e) {
      return -1;
    }
    ExportStat e = new ExportStat(taskId, fileName, startDate);
    synchronized(lock) {
      queue.addLast(e);
    }
    return 1;
  }

  /** @noinspection EmptyCatchBlock*/
  public void run() {
    started = true;
    if (logger.isDebugEnabled()) {
      logger.debug("InfoSmeExportStatManager is started");
    }
    while(started) {
      try{
        ExportStat e = null;
        synchronized(lock) {
          if(!queue.isEmpty()) {
            e = (ExportStat)queue.removeFirst();
          }
        }
        if(e != null) {
          synchronized(shutdownLock) {
            export(e);
          }
        } else {
          try {
            Thread.sleep(5000);
          } catch (InterruptedException ex) {}
        }
      }catch(Throwable e) {
        logger.error(e,e);
      }
    }
  }

  public void start() {
    if (!started) {
      new Thread(this, "InfoSmeExportStatManager").start();
    }
  }

  public void shutdown() {
    started = false;
    synchronized(shutdownLock){
      if(logger.isDebugEnabled()) {
        logger.debug("InfoSmeExportStatManager is shutdowned");
      }
    }
  }

  private static File createTmpFile(File file) throws IOException {
    File tmp = new File(file.getAbsolutePath()+".stat.tmp");
    File p = tmp.getParentFile();
    if(p != null && !p.exists()) {
      if(!p.mkdirs()) {
        throw new IOException("Can't create tmp file: "+tmp.getAbsolutePath());
      }
    }
    if(!tmp.createNewFile()) {
      throw new IOException("Can't create tmp file: "+tmp.getAbsolutePath());
    }
    tmp.deleteOnExit();
    return tmp;
  }

  /** @noinspection EmptyCatchBlock*/
  public void export(ExportStat exportStat) {
    if(logger.isDebugEnabled()) {
      logger.debug("Start to export statistics: "+exportStat);
    }
    try{
      final ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
      final PrintWriter[] errorStream = new PrintWriter[1];
      MessageFilter messageFilter = new MessageFilter();
      messageFilter.setTaskId(exportStat.taskId);
      final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy HH:mm");
      if(exportStat.startDate != null) {
        messageFilter.setFromDate(exportStat.startDate);
        messageFilter.setFromDateEnabled(true);
      }
      try{
        errorStream[0]= new PrintWriter(byteArrayOutputStream);

        final BufferedWriter[] writer = new BufferedWriter[1];

        MessageVisitor mV = new MessageVisitor(){
          public boolean visit(MessageDataItem msg){
            try{
              Object o;
              writer[0].write((o = msg.getValue(MessageDataSource.MSISDN)) !=null ? (String)o : "");
              writer[0].write(',');
              writer[0].write((o = msg.getValue(MessageDataSource.STATE)) != null && ((Message.State)o).getName() != null ? ((Message.State)o).getName() : "");
              writer[0].write(',');
              writer[0].write((o = msg.getValue(MessageDataSource.DATE)) != null  ? sdf.format((Date)o) : "");
              writer[0].write(',');
              writer[0].write((o = msg.getValue(MessageDataSource.MESSAGE)) != null ? o.toString() : "");
              writer[0].write(System.getProperty("line.separator"));
              return true;
            }catch(IOException e) {
              logger.error(e,e);
              e.printStackTrace(errorStream[0]);
              return false;
            }
          }
        };
        try{
          writer[0] = new BufferedWriter(new FileWriter(exportStat.tmp));
          ds.visit(mV, messageFilter);
        } finally{
          if(writer[0] != null) {
            try{
              writer[0].close();
            }catch(Exception e){}
          }
        }
      }catch(Exception e) {
        logger.error(e,e);
        if(errorStream[0] != null) {
          e.printStackTrace(errorStream[0]);
        }
      }finally{
        if(errorStream[0] != null) {
          try{
            errorStream[0].close();
          }catch(Exception e){}
        }
      }

      byte[] e = byteArrayOutputStream.toByteArray();
      if(e.length != 0 ) {
        writeError(exportStat.tmp, e);
        exportStat.tmp.renameTo(new File(exportStat.file+".stat.failed"));
      }else {
        if(!exportStat.tmp.renameTo(new File(exportStat.file.getAbsolutePath()+".stat"))) {
          String errorStr =
              new StringBuffer(200).append("Can't rename file: ").append(exportStat.tmp.getAbsolutePath()).
                  append(" to ").append(exportStat.file.getAbsolutePath()).toString();
          logger.error(errorStr);
          writeError(exportStat.tmp, errorStr);
          exportStat.tmp.renameTo(new File(exportStat.file+".stat.failed"));
        }else {
          if(logger.isDebugEnabled()) {
            logger.debug("Exporting is finished: "+exportStat);
          }
        }
      }
    }catch(Exception e) {
      logger.error(e,e);
    }
  }

  private static void writeError(File f, String s) {
    writeError(f, s.getBytes());
  }

  /** @noinspection EmptyCatchBlock*/
  private static void writeError(File f, byte[] e) {
    OutputStream writer = null;
    try {
      writer = new BufferedOutputStream(new FileOutputStream(f));
      writer.write(e);
    }catch(Exception ex){
      logger.error(ex,ex);
    }finally{
      if(writer != null) {
        try{
          writer.close();
        }catch(IOException ex) {}
      }
    }
  }

  private static class ExportStat {

    private File file;

    private File tmp;

    private Date startDate;

    private String taskId;

    public ExportStat(String taskId, String fileName, Date startDate) throws IOException {
      if(taskId == null || fileName == null) {
        throw new IllegalArgumentException();
      }
      this.taskId = taskId;
      this.file = new File(fileName);
      this.startDate = startDate;
      tmp = createTmpFile(file);
    }
    public String toString() {
      return new StringBuffer(150).append("taskId='").append(taskId).append("'\n").
          append("outputFile='").append(file.getAbsolutePath()).append("'\n").
          append("startDate='").append(startDate).append('\'').toString();

    }
  }


}
