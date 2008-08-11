package ru.novosoft.smsc.infosme.beans.deliveries;

import ru.novosoft.smsc.infosme.backend.Task;
import ru.novosoft.smsc.infosme.backend.MultiTask;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

/**
 * User: artem
 * Date: 04.06.2008
 */

public class ProcessFilePage extends DeliveriesPage {

  private final LoadDeliveriesFileThread thread;

  public ProcessFilePage(DeliveriesPageData pageData) {
    super(pageData);
    thread = new LoadDeliveriesFileThread(pageData.getDeliveriesFile(), pageData.getInfoSmeContext(), pageData.isSplitDeliveriesFile(), pageData.getAppContext());
    thread.start();
  }

  public DeliveriesPage mbNext(HttpServletRequest request) throws AdminException {

    switch(thread.getStatus()) {
      case SplitDeliveriesFileThread.STATUS_DONE:
        HashMap outputFiles = thread.getOutputFiles();

        if (outputFiles.isEmpty())
          return new StartPage(pageData);

        final HashMap inputFiles = new HashMap(outputFiles.size());
        final MultiTask task = new MultiTask();
        try {
          task.setAddress(pageData.getInfoSmeContext().getConfig().getString("InfoSme.Address"));
        } catch (Config.ParamNotFoundException e) {
          throw new AdminException(e.getMessage());
        } catch (Config.WrongParamTypeException e) {
          throw new AdminException(e.getMessage());
        }

        Map.Entry e;
        for (Iterator iter = outputFiles.entrySet().iterator(); iter.hasNext();) {
          e = (Map.Entry)iter.next();
          String region = (String)e.getKey();
          SplitDeliveriesFileThread.RegionOutputFile outputFile = (SplitDeliveriesFileThread.RegionOutputFile)e.getValue();

          final Task t = pageData.getInfoSmeContext().getTaskManager().createTask();
          resetTask(t, request);
          t.setSubject(region);
          t.setActualRecordsSize(outputFile.getTotalSize());

          task.addTask(region, t);

          inputFiles.put(region, outputFile.getFile());

          if (pageData.activeTaskSubject == null)
            pageData.activeTaskSubject = region;
        }

        pageData.setInputFiles(inputFiles);

        pageData.setTask(task);

        pageData.oldActiveTaskSubject = pageData.activeTaskSubject;

        return new EditTaskPage(pageData);

      case SplitDeliveriesFileThread.STATUS_CANCELED:
      case SplitDeliveriesFileThread.STATUS_ERROR:
        return new StartPage(pageData);
      default:
        return this;
    }
  }

  public DeliveriesPage mbCancel(HttpServletRequest request) throws AdminException {
    if (thread.getStatus() == SplitDeliveriesFileThread.STATUS_PROCESSING || thread.getStatus() == SplitDeliveriesFileThread.STATUS_INITIALIZATION) {
      thread.shutdown();
      return this;
    }

    return new StartPage(pageData);
  }

  public DeliveriesPage mbUpdate(HttpServletRequest request) throws AdminException {
    LoadDeliveriesFileThread.Progress progress = thread.getProgress();
    pageData.recordsProcessed = progress.getRecordsProcessed();
    pageData.unrecognized = progress.getUnrecognized();
    pageData.inblackList = progress.getInblackList();
    pageData.subjectsFound = progress.getSubjectsFound();
    pageData.splitDeliveriesFileStatus = thread.getStatus();
    return this;
  }

  public static boolean isUserAdmin(HttpServletRequest request) {
    return request.isUserInRole("infosme-admin");
  }

  private static void resetTask(Task task, HttpServletRequest request) {
    task.setDelivery(true);
    task.setProvider(Task.INFOSME_EXT_PROVIDER);
    task.setPriority(10);
    task.setMessagesCacheSize(2000);
    task.setMessagesCacheSleep(11);
    task.setUncommitedInGeneration(100);
    task.setUncommitedInProcess(100);
    task.setEnabled(true);
    task.setTrackIntegrity(true);
    task.setKeepHistory(true);
    task.setReplaceMessage(false);
    task.setRetryOnFail(false);
    task.setRetryTime("03:00:00");
    task.setSvcType("dlvr");
    task.setActivePeriodStart("10:00:00");
    task.setActivePeriodEnd("21:00:00");
    task.setTransactionMode(false);
    task.setStartDate(new Date());
    task.setValidityPeriod(isUserAdmin(request) ? "01:00:00" : "00:45:00");
  }

  public int getId() {
    return PROCESS_FILE_PAGE;
  }
}
