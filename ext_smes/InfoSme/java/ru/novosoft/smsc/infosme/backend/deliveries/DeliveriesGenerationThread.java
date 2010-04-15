package ru.novosoft.smsc.infosme.backend.deliveries;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.infosme.backend.MultiTask;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;

import java.io.*;
import java.util.*;

import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 04.06.2008
 */

public class DeliveriesGenerationThread extends Thread {

  private static final Category log = Category.getInstance(DeliveriesGenerationThread.class);

  public final static int STATUS_INITIALIZATION = 0;
  public final static int STATUS_GENERATION = 1;
  public final static int STATUS_DONE = 2;
  public final static int STATUS_ERR = 3;
  public final static int STATUS_CANCELED = 4;

  private final InfoSmeContext infoSmeContext;
  private final MultiTask task;
  private final HashMap generationProgress;

  private int status;
  private String errorText;


  private boolean started = true;

  public DeliveriesGenerationThread(InfoSmeContext infoSmeContext, MultiTask task) {
    this.infoSmeContext = infoSmeContext;
    this.task = task;

    generationProgress = new HashMap(task.tasks().size());
  }

  public int getStatus() {
    return status;
  }

  public String getErrorText() {
    return errorText;
  }

  public HashMap getProgress() {
    return generationProgress;
  }

  private void removeTask() throws AdminException {
    for (Iterator iter = task.tasks().iterator(); iter.hasNext();) {
      Task t = (Task)iter.next();
      if (t != null) {
        infoSmeContext.getInfoSmeConfig().removeAndApplyTask(t.getOwner(), t.getId());
        try {
          infoSmeContext.getInfoSme().removeTask(t.getId());
        } catch (Throwable e) {
          e.printStackTrace();
        }
      }
    }
  }

  private void rollback() {
    try {
      removeTask();
    } catch (Exception e) {
      log.error(e,e);
    }
  }

  public void run() {

    status = STATUS_INITIALIZATION;
    try {
      // Store tasks to config file
      for (Iterator iter = task.tasks().iterator(); iter.hasNext();) {
        Task t = (Task)iter.next();
        infoSmeContext.getInfoSmeConfig().addAndApplyTask(t);
      }

      // Add tasks to info sme
      for (Iterator iter = task.tasks().iterator(); iter.hasNext();)
        infoSmeContext.getInfoSme().addTask(((Task)iter.next()).getId());


      final int maxMessagesPerSecond = infoSmeContext.getLicense().getMaxSmsThroughput();

      // Fill genInfos
      for (Iterator iter = task.tasks().iterator(); iter.hasNext();) {
        Task t = (Task)iter.next();
        Progress p = new Progress(t.getActualRecordsSize());
        generationProgress.put(new Integer(t.getRegionId()), p);
      }

      /// Generate messages for tasks
      status = STATUS_GENERATION;

      // Process each input file
      for (Iterator iter = task.iterator(); iter.hasNext();) {
        Task t = (Task)iter.next();
        Progress progress = (Progress)generationProgress.get(new Integer(t.getRegionId()));
        addDeliveries(t, progress, maxMessagesPerSecond);
      }

      status = (started) ? STATUS_DONE : STATUS_CANCELED;

    } catch (Throwable e) {
      e.printStackTrace();
      rollback();
      status = STATUS_ERR;
      errorText = e.getMessage();
    }
  }

  private void addDeliveries(Task t, Progress progress, int maxMessagesPerSecond) throws IOException, AdminException {

    BufferedReader bis = null;
    try {
      bis = new BufferedReader(new FileReader(t.getDeliveriesFile()));

      Date curTime = t.getStartDate();
      if (curTime == null)
        curTime = new Date();
      String line = null;
      ArrayList messages = new ArrayList(maxMessagesPerSecond);

      while (started) {
        messages.clear();

        // Prepare array of messages
        int i=0;
        while (started && i < maxMessagesPerSecond && (line = bis.readLine()) != null) {
          final Message message = new Message();
          message.setState(Message.State.NEW);
          message.setSendDate(curTime);

          String msisdn, text;
          if (t.isDeliveriesFileContainsTexts()) {
            int ind= line.indexOf('|');
            msisdn = line.substring(0, ind);
            text = line.substring(ind + 1);
            text.replaceAll("\\\\r", "").replaceAll("\\\\n", System.getProperty("line.separator"));
          } else {
            msisdn = line;
            text = t.getText();
          }
          message.setMessage(text);
          message.setAbonent(msisdn);

          messages.add(message);

          progress.incProgress();
          i++;
        }

        if (started && !messages.isEmpty()) {
          infoSmeContext.getInfoSme().addDeliveryMessages(t.getId(), messages);
          curTime = new Date(curTime.getTime() + 1000);
        }

        if (line == null)
          break;
      }

    } finally {
      if (bis != null)
        try {
          bis.close();
        } catch (IOException e1) {
          log.error(e1,e1);
        }
    }
  }

  public void cancel() {
    started = false;
    status = STATUS_CANCELED;
    rollback();
  }


  public static class Progress {
    private int total;
    private int msgCount;
    private int progressPercent;

    Progress(int total) {
      this.total = total;
    }

    public void incProgress() {
      msgCount++;
      progressPercent = msgCount * 100 / total;
    }

    public int getMsgCount() {
      return msgCount;
    }

    public double getProgressPercent() {
      return progressPercent;
    }
  }
}
