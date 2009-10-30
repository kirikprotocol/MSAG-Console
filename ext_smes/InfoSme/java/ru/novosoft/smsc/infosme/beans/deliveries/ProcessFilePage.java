package ru.novosoft.smsc.infosme.beans.deliveries;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.admin.region.Region;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.MultiTask;
import ru.novosoft.smsc.infosme.backend.deliveries.LoadDeliveriesFileThread;
import ru.novosoft.smsc.infosme.backend.deliveries.DeliveriesFile;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.jsp.SMSCAppContext;

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
    thread = new LoadDeliveriesFileThread(pageData.getDeliveriesFile(), pageData.getInfoSmeContext(), pageData.isSplitDeliveriesFile(), pageData.getAppContext(), pageData.owner);
    thread.start();
  }

  public DeliveriesPage mbNext(HttpServletRequest request) throws AdminException {

    switch(thread.getStatus()) {
      case LoadDeliveriesFileThread.STATUS_DONE:
        HashMap outputFiles = thread.getOutputFiles();

        if (outputFiles.isEmpty())
          return new LoadFilePage(pageData);

//        final HashMap inputFiles = new HashMap(outputFiles.size());
        final MultiTask multiTask = new MultiTask();

        Map.Entry e;
        for (Iterator iter = outputFiles.entrySet().iterator(); iter.hasNext();) {
          e = (Map.Entry)iter.next();
          int region = ((Integer)e.getKey()).intValue();
          DeliveriesFile deliveriesFile = (DeliveriesFile)e.getValue();

          final Task t = pageData.getInfoSmeContext().getInfoSmeConfig().createTask();
          t.setRegionId(region);
          t.setActualRecordsSize(deliveriesFile.getTotalSize());
          t.setOwner(pageData.owner.getName());
          resetTask(t, pageData.getInfoSmeContext(), pageData.getAppContext(), pageData.owner);

          multiTask.addTask(region, t);
          t.setDeliveriesFile(deliveriesFile.getFile());
          t.setDeliveriesFileContainsTexts(deliveriesFile.isContainsTexts());

//          inputFiles.put(new Integer(region), deliveriesFile.getFile());

          if (pageData.activeTaskRegionId == -1)
            pageData.activeTaskRegionId = region;
        }

//        pageData.setInputFiles(inputFiles);

        pageData.setTask(multiTask);

        pageData.oldActiveTaskRegionId = pageData.activeTaskRegionId;

        return new EditTaskPage(pageData);

      case LoadDeliveriesFileThread.STATUS_CANCELED:
      case LoadDeliveriesFileThread.STATUS_ERROR:
        return new LoadFilePage(pageData);
      default:
        return this;
    }
  }

  public DeliveriesPage mbCancel(HttpServletRequest request) throws AdminException {
    if (thread.getStatus() == LoadDeliveriesFileThread.STATUS_PROCESSING || thread.getStatus() == LoadDeliveriesFileThread.STATUS_INITIALIZATION) {
      thread.shutdown();
      return this;
    }

    return new LoadFilePage(pageData);
  }

  public DeliveriesPage mbUpdate(HttpServletRequest request) throws AdminException {
    LoadDeliveriesFileThread.Progress progress = thread.getProgress();
    pageData.recordsProcessed = progress.getRecordsProcessed();
    pageData.unrecognized = progress.getUnrecognized();
    pageData.inblackList = progress.getInblackList();
    pageData.regionsFound = progress.getSubjectsFound();
    pageData.splitDeliveriesFileStatus = thread.getStatus();
    pageData.splitDeliveriesFileStatusStr = thread.getStatusStr();
    return this;
  }

  public static boolean isUserAdmin(HttpServletRequest request) {
    return request.isUserInRole("infosme-admin");
  }

  private static void resetTask(Task task, InfoSmeContext context, SMSCAppContext appContext, User owner) throws AdminException {

    task.setAddress(context.getInfoSmeConfig().getAddress());
    task.setDelivery(true);
    task.setProvider(Task.INFOSME_EXT_PROVIDER);
    task.setPriority(owner.getPrefs().getInfosmePriority());
    task.setMessagesCacheSize(owner.getPrefs().getInfosmeCacheSize());
    task.setMessagesCacheSleep(owner.getPrefs().getInfosmeCacheSleep());
    task.setUncommitedInGeneration(owner.getPrefs().getInfosmeUncommitGeneration());
    task.setUncommitedInProcess(owner.getPrefs().getInfosmeUncommitProcess());
    task.setTrackIntegrity(owner.getPrefs().isInfosmeTrackIntegrity());
    task.setKeepHistory(owner.getPrefs().isInfosmeKeepHistory());
    task.setReplaceMessage(owner.getPrefs().isInfosmeReplaceMessage());
    task.setSvcType(owner.getPrefs().getInfosmeSvcType());
    task.setEnabled(true);

    Region r = appContext.getRegionsManager().getRegionById(task.getRegionId());
    task.setActivePeriodStart(r == null ? owner.getPrefs().getInfosmePeriodStart() : r.getLocalTime(owner.getPrefs().getInfosmePeriodStart()));
    task.setActivePeriodEnd(r == null ? owner.getPrefs().getInfosmePeriodEnd() : r.getLocalTime(owner.getPrefs().getInfosmePeriodEnd()));
    task.setValidityPeriod(owner.getPrefs().getInfosmeValidityPeriod());
    task.setTransactionMode(owner.getPrefs().isInfosmeTrMode());
    task.setActiveWeekDaysSet(owner.getPrefs().getInfosmeWeekDaysSet());

    task.setStartDate(new Date());
  }

  public int getId() {
    return PROCESS_FILE_PAGE;
  }
}
