package ru.sibinco.smpp.textservice;

import ru.aurorisoft.smpp.*;

import java.util.*;
import java.io.*;
import java.text.SimpleDateFormat;

/**
 * Created by IntelliJ IDEA.
 * User: kpv
 * Date: 03.10.2005
 * Time: 12:57:52
 * To change this template use File | Settings | File Templates.
 */
public class StatisticsCollectorStub implements StatisticsCollector {

  private final static org.apache.log4j.Category Logger = org.apache.log4j.Category.getInstance(StatisticsCollectorStub.class);

  private static final String outputDirSuffix = "stats.collector.output.dir";
  private String outputDir = null;
  private Map outFiles = new HashMap();
  private SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy HH:mm");

  public void init(String s, String s1) throws SMPPException {
  }

  public void init(Properties properties, String s) throws SMPPException {
    outputDir = properties.getProperty(outputDirSuffix, "");
    Logger.info("StatisticsCollectorStub initialized.");
  }

  public void registerOutMessage(Message message) throws SMPPException {
  }

  public void registerInMessage(Message message) throws SMPPException {
    if (outputDir.equals(""))
      return;
    try {
      DatedPrintWriter dpw = (DatedPrintWriter)outFiles.get(message.getDestinationAddress());
      File f = new File(outputDir+System.getProperty("file.separator")+message.getDestinationAddress()+"_"+Long.toString(getCurrentRdate())+".log");
      if (dpw != null && (dpw.getDate() != getCurrentRdate() || !f.exists())) {
        dpw.getPrintWriter().close();
        dpw= null;
      }
      if (dpw == null) {
        dpw = new DatedPrintWriter(getCurrentRdate(), new PrintWriter(new FileOutputStream(f, true)));
        outFiles.put(message.getDestinationAddress(), dpw);
      }
      dpw.getPrintWriter().print(message.getSourceAddress());
      dpw.getPrintWriter().print(",");
      dpw.getPrintWriter().print(sdf.format(new Date()));
      dpw.getPrintWriter().print(",");
      dpw.getPrintWriter().println(message.getMessageString());
      dpw.getPrintWriter().flush();
    } catch (Exception e) {
       Logger.error("Could not save request.", e);
    }
  }

  public void registerErrMessage(String s, Throwable throwable) throws SMPPException {
  }

  public void shutdown() throws SMPPException {
    for (Iterator iterator = outFiles.keySet().iterator(); iterator.hasNext();) {
      String key = (String)iterator.next();
      try {
        DatedPrintWriter dpw = (DatedPrintWriter)outFiles.get(key);
        dpw.getPrintWriter().close();
      } catch (Exception e) {
        Logger.warn("Could not close output file for "+key, e);
      }
    }
  }

  protected long getCurrentRdate() {
    Calendar cal = Calendar.getInstance();
    long rdate = cal.get(Calendar.YEAR) * 10000L + (cal.get(Calendar.MONTH) + 1) * 100 + cal.get(Calendar.DAY_OF_MONTH);
    return rdate;
  }

  class DatedPrintWriter {
    private long date = -1;
    private PrintWriter printWriter = null;

    public DatedPrintWriter(long date, PrintWriter printWriter) {
      this.date = date;
      this.printWriter = printWriter;
    }

    public long getDate() {
      return date;
    }

    public PrintWriter getPrintWriter() {
      return printWriter;
    }
  }
}
