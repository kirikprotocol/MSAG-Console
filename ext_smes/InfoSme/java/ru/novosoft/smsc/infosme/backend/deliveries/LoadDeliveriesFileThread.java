package ru.novosoft.smsc.infosme.backend.deliveries;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.region.Region;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.BlackListManager;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.radixtree.TemplatesRadixTree;
import ru.novosoft.smsc.infosme.beans.InfoSmeBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.WebAppFolders;

import java.io.*;
import java.util.*;

/**
 * User: artem
 * Date: 11.08.2008
 */

public class LoadDeliveriesFileThread extends Thread {

  private static final Category log = Category.getInstance(LoadDeliveriesFileThread.class);

  public static final int STATUS_INITIALIZATION = 0;
  public static final int STATUS_PROCESSING = 1;
  public static final int STATUS_DONE = 2;
  public static final int STATUS_ERROR = 3;
  public static final int STATUS_CANCELED = 4;

  private final InfoSmeContext infoSmeContext;
  private final SMSCAppContext appContext;
  private final Progress progress;
  private final HashMap outputFiles;
  private final File inputFile;
  private final boolean splitFile;
  private final User user;

  private volatile boolean started = true;
  private volatile int status = 0;
  private String statusStr;

  public LoadDeliveriesFileThread(File inputFile, InfoSmeContext infoSmeContext, boolean splitFile, SMSCAppContext appContext, User user) {
    this.infoSmeContext = infoSmeContext;
    this.appContext = appContext;
    this.progress = new Progress();
    this.outputFiles = new HashMap();
    this.inputFile = inputFile;
    this.splitFile = splitFile;
    this.user = user;
  }

  public void shutdown() {
    started = false;
  }

  public int getStatus() {
    return status;
  }

  public String getStatusStr() {
    return statusStr;
  }

  public Progress getProgress() {
    return progress;
  }

  public HashMap getOutputFiles() {
    return outputFiles;
  }

  public void run() {

    try {
      this.status = STATUS_INITIALIZATION;
      // Initiate radix tree
      TemplatesRadixTree radixTree = new TemplatesRadixTree();
      initiateRadixTree(radixTree, splitFile);

      // Init black list manager
      final BlackListManager blm = infoSmeContext.getBlackListManager();

      this.status = STATUS_PROCESSING;
      BufferedReader is = null;
      try {
        is = new BufferedReader(new FileReader(inputFile));

        String line;
        Boolean containsText = null;
        DeliveriesFile deliveriesFile;
        while (started && (line = is.readLine()) != null) {
          progress.recordsProcessed++;

          line = line.trim();
          if (line.length() == 0)
            continue;

          int i= line.indexOf("|");

          if (containsText != null) {
              if (containsText.booleanValue() != i > 0)
                throw new AdminException("Invalid file format");
            } else
              containsText = Boolean.valueOf(i > 0);

          if (!line.startsWith("+"))
            line = '+' + line;

          String msisdn = i > 0 ? line.substring(0, i).trim() : line;

          if (blm.contains(msisdn)) {
            progress.inblackList++;
            continue;
          }

          deliveriesFile = (DeliveriesFile)radixTree.getValue(msisdn);
          if (deliveriesFile != null) {
            // Add MSISDN to file
            deliveriesFile.addLine(line);
            deliveriesFile.setContainsTexts(containsText.booleanValue());

            if (!outputFiles.containsKey(new Integer(deliveriesFile.getRegion()))) {
              outputFiles.put(new Integer(deliveriesFile.getRegion()), deliveriesFile);
              progress.subjectsFound++;
            }
          } else {
            progress.unrecognized++;
            log.warn("No region found for " + msisdn);
          }
        }

      } finally {
        try {
          if (is != null)
            is.close();
        } catch (IOException e) {
          log.error(e,e);
        }

        for (Iterator iter = outputFiles.values().iterator(); iter.hasNext();) {
          try {
            ((DeliveriesFile)iter.next()).close();
          } catch (IOException e) {
            log.error(e,e);
          }
        }
      }

      this.status = (started) ? STATUS_DONE : STATUS_CANCELED;

    } catch (Throwable e) {
      e.printStackTrace();
      this.statusStr = e.getMessage();
      this.status = STATUS_ERROR;
    }
  }

  private void initiateRadixTree(TemplatesRadixTree tree, boolean split) {
    Collection regions = appContext.getRegionsManager().getRegions();
    boolean admin = user.getRoles().contains(InfoSmeBean.INFOSME_ADMIN_ROLE);

    DeliveriesFile defaultFile = split ? null : new DeliveriesFile(-1, Functions.createNewFilenameForSave(new File(WebAppFolders.getWorkFolder(), "INFO_SME_abonents.region_ALL.list")));

    Region r;
    for (Iterator regionsIter = regions.iterator(); regionsIter.hasNext();) {
      r = (Region)regionsIter.next();
      if (admin || user.getPrefs().isInfoSmeRegionAllowed(String.valueOf(r.getId()))) {

        DeliveriesFile outFile;
        if (split) {
          File f = Functions.createNewFilenameForSave(new File(WebAppFolders.getWorkFolder(), "INFO_SME_abonents.region_" + r.getId() + ".list"));
          outFile = new DeliveriesFile(r.getId(), f);
        } else
          outFile = defaultFile;

        for (Iterator subjectsIter = r.getSubjects().iterator(); subjectsIter.hasNext();) {
          String subjectName = (String)subjectsIter.next();
          Subject s = appContext.getRouteSubjectManager().getSubjects().get(subjectName);
          tree.add(s.getMasks().getNames(), outFile);
        }
      }
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
