package ru.novosoft.smsc.infosme.beans.deliveries;

import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.admin.region.Region;
import ru.novosoft.smsc.infosme.backend.BlackListManager;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.radixtree.TemplatesRadixTree;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.WebAppFolders;

import java.io.*;
import java.util.*;

import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 04.06.2008
 */

public class SplitDeliveriesFileThread extends Thread {

  private static final Category log = Category.getInstance(SplitDeliveriesFileThread.class);

  public static final int STATUS_INITIALIZATION = 0;
  public static final int STATUS_PROCESSING = 1;
  public static final int STATUS_DONE = 2;
  public static final int STATUS_ERROR = 3;
  public static final int STATUS_CANCELED = 4;

  private final static int ESC_CR = 13;
  private final static int ESC_LF = 10;

  private final InfoSmeContext infoSmeContext;
  private final SMSCAppContext appContext;
  private final Progress progress;
  private final HashMap outputFiles;
  private final File inputFile;

  private volatile boolean started = true;
  private volatile int status = 0;

  public SplitDeliveriesFileThread(File inputFile, InfoSmeContext infoSmeContext, SMSCAppContext appContext) {
    this.infoSmeContext = infoSmeContext;
    this.appContext = appContext;
    this.progress = new Progress();
    this.outputFiles = new HashMap();
    this.inputFile = inputFile;
  }

  public void shutdown() {
    started = false;
  }

  public int getStatus() {
    return status;
  }

  public Progress getProgress() {
    return progress;
  }

  public HashMap getOutputFiles() {
    return outputFiles;
  }

  private static String readLine(InputStream bis) throws IOException {
    int ch;
    final StringBuffer sb = new StringBuffer(30);
    while (true) {
      ch = bis.read();
      if (ch == -1 || ch == ESC_LF)
        break;
      else if (ch != ESC_CR)
        sb.append((char)ch);
    }
    return (sb.length() == 0) ? null : sb.toString().trim();
  }

  private TemplatesRadixTree initRadixTree() {
    final TemplatesRadixTree regionsTree = new TemplatesRadixTree();
    // Fill regions tree
    Collection regions = appContext.getRegionsManager().getRegions();
    Region r;
    for (Iterator regionsIter = regions.iterator(); regionsIter.hasNext();) {
      r = (Region)regionsIter.next();

      RegionOutputFile outFile = new RegionOutputFile(r.getName(), Functions.createNewFilenameForSave(new File(WebAppFolders.getWorkFolder(), "INFO_SME_abonents." + r.getName() + ".list")));

      for (Iterator subjectsIter = r.getSubjects().iterator(); subjectsIter.hasNext();) {
        String subjectName = (String)subjectsIter.next();
        Subject s = appContext.getRouteSubjectManager().getSubjects().get(subjectName);
        regionsTree.add(s.getMasks().getNames(), outFile);
      }

    }
    return regionsTree;
  }

  public void run() {
    this.status = STATUS_INITIALIZATION;

    // Init black list manager
    final BlackListManager blm = infoSmeContext.getBlackListManager();
    // Init subjects tree
    final TemplatesRadixTree radixTree = initRadixTree();

    this.status = STATUS_PROCESSING;

    InputStream is = null;
    try {
      is = new BufferedInputStream(new FileInputStream(inputFile));

      String msisdn;
      RegionOutputFile outputFile;
      while ((msisdn = readLine(is)) != null && started) {
        progress.recordsProcessed++;

        if (!msisdn.startsWith("+"))
          msisdn = '+' + msisdn;

        if (blm.contains(msisdn)) {
          progress.inblackList++;
          continue;
        }

        outputFile = (RegionOutputFile)radixTree.getValue(msisdn);
        if (outputFile != null) {
          // Add MSISDN to file
          outputFile.addMsisdn(msisdn);
          if (!outputFiles.containsKey(outputFile.region)) {
            outputFiles.put(outputFile.region, outputFile);
            progress.subjectsFound++;
          }
        } else {
          progress.unrecognized++;
          log.warn("No region found for " + msisdn);
        }
      }

      this.status = (started) ? STATUS_DONE : STATUS_CANCELED;

    } catch (Exception e) {
      e.printStackTrace();
      this.status = STATUS_ERROR;
      return;

    } finally {
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {
      }

      for (Iterator iter = outputFiles.values().iterator(); iter.hasNext();) {
        try {
          ((RegionOutputFile)iter.next()).close();
        } catch (IOException e) {
        }
      }
    }
  }

  public class RegionOutputFile {

    private final String region;
    private final File file;
    private OutputStream os;
    private final ArrayList buffer;

    private int totalSize;

    private RegionOutputFile(String region, File file) {
      this.region = region;
      this.file = file;
      this.os = null;
      this.buffer = new ArrayList(1000);
    }

    private void addMsisdn(String msisdn) throws IOException {
      buffer.add(msisdn);
      totalSize++;
      if (buffer.size()>=1000)
        flushBuffer();
    }

    private void flushBuffer() throws IOException {
      if (os == null) {
        os = new BufferedOutputStream(new FileOutputStream(file));
        file.deleteOnExit();
      }

      Collections.shuffle(buffer);

      for (int i=0; i < buffer.size(); i++) {
        os.write(((String)buffer.get(i)).getBytes());
        os.write(ESC_LF);
      }

      buffer.clear();
    }

    private void close() throws IOException {
      flushBuffer();
      os.close();
    }

    public int getTotalSize() {
      return totalSize;
    }

    public File getFile() {
      return file;
    }

    public String getRegion() {
      return region;
    }
  }

  public static class Progress {

    private int recordsProcessed;
    private int subjectsFound;
    private int unrecognized;
    private int inblackList;

    private Progress() {
    }

    public void incReconrdProcessed() {
      recordsProcessed++;
    }

    public int getRecordsProcessed() {
      return recordsProcessed;
    }

    public void incSubjectsFound() {
      subjectsFound++;
    }

    public int getSubjectsFound() {
      return subjectsFound;
    }

    public void incUnrecognized() {
      unrecognized++;
    }

    public int getUnrecognized() {
      return unrecognized;
    }

    public void incInBlackList() {
      inblackList++;
    }

    public int getInblackList() {
      return inblackList;
    }
  }
}
