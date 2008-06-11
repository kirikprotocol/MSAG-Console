package ru.novosoft.smsc.infosme.beans.deliveries;

import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.infosme.backend.MultiTask;
import ru.novosoft.smsc.infosme.backend.Task;
import ru.novosoft.smsc.util.config.Config;

import java.io.*;
import java.util.*;

/**
 * User: artem
 * Date: 04.06.2008
 */

public class DeliveriesGenerationThread extends Thread {

  public final static int STATUS_INITIALIZATION = 0;
  public final static int STATUS_GENERATION = 1;
  public final static int STATUS_DONE = 2;
  public final static int STATUS_ERR = 3;
  public final static int STATUS_CANCELED = 4;

  private final static int ESC_CR = 13;
  private final static int ESC_LF = 10;

  private final InfoSmeContext infoSmeContext;
  private final HashMap inputFiles;
  private final MultiTask task;
  private final HashMap generationProgress;

  private int status;
  private String errorText;

  private boolean started = true;

  public DeliveriesGenerationThread(InfoSmeContext infoSmeContext, HashMap inputFiles, MultiTask task) {
    this.infoSmeContext = infoSmeContext;
    this.inputFiles = inputFiles;
    this.task = task;

    generationProgress = new HashMap(task.tasks().size());
  }

  public int getStatus() {
    return status;
  }

  public String getErrorText() {
    return errorText;
  }

  public HashMap getGenerationProgress() {
    return generationProgress;
  }

  private void removeTask() throws Exception {
    infoSmeContext.resetConfig();
    for (Iterator iter = task.tasks().iterator(); iter.hasNext();) {
      Task t = (Task)iter.next();
      if (t != null) {
        System.out.println("Remove " + t.getName() + " from config: " + t.getId());
        t.removeFromConfig(infoSmeContext.getConfig());
        infoSmeContext.getConfig().save();
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
    }
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

  public void run() {

    status = STATUS_INITIALIZATION;
    final Config oldConfig = infoSmeContext.getConfig();
    try {
      // Store tasks to config file
      task.storeToConfig(oldConfig);
      oldConfig.save();

      // Add tasks to info sme
      for (Iterator iter = task.tasks().iterator(); iter.hasNext();)
        infoSmeContext.getInfoSme().addTask(((Task)iter.next()).getId());


      final int maxMessagesPerSecond = infoSmeContext.getConfig().getInt("InfoSme.maxMessagesPerSecond");      

      // Generate messages for tasks
      status = STATUS_GENERATION;

      // Fill genInfos
      for (Iterator iter = task.tasks().iterator(); iter.hasNext();) {
        Task t = (Task)iter.next();
        DeliveriesGenerationProgress p = new DeliveriesGenerationProgress(t.getActualRecordsSize());
        generationProgress.put(t.getSubject(), p);
      }

      // Process each input file
      Map.Entry e;
      String subject;
      File file;
      DeliveriesGenerationProgress progress;
      for (Iterator iter = inputFiles.entrySet().iterator(); iter.hasNext() && started;) {
        e = (Map.Entry)iter.next();

        subject = (String)e.getKey();
        file = (File)e.getValue();
        progress = (DeliveriesGenerationProgress)generationProgress.get(subject);

        InputStream bis = null;
        try {
          bis = new BufferedInputStream(new FileInputStream(file));

          Task t = task.getTask(subject);
          Date curTime = (t.getStartDateDate() == null) ? new Date() : t.getStartDateDate();
          String msisdn = null;
          while (started) {
            ArrayList messages = new ArrayList(maxMessagesPerSecond);
            int i=0;
            while (i < maxMessagesPerSecond && (msisdn = readLine(bis)) != null && started) {
              final Message message = new Message();
              message.setState(Message.State.NEW);
              message.setSendDate(curTime);
              message.setMessage(t.getText());
              Mask address = new Mask(msisdn);
              message.setAbonent(address.getMask());

              messages.add(message);

              progress.incProgress();
              i++;
            }

            if (started && messages.size() > 0) {
              infoSmeContext.getInfoSme().addDeliveryMessages(t.getId(), messages);
              curTime = new Date(curTime.getTime() + 1000);
            }

            if (msisdn == null)
              break;
          }

        } finally {
          if (bis != null)
            try {
              bis.close();
            } catch (IOException e1) {
            }
        }
      }
      status = (started) ? STATUS_DONE : STATUS_CANCELED;

    } catch (Throwable e) {
      e.printStackTrace();
      rollback();
      status = STATUS_ERR;
      errorText = e.getMessage();
    }
  }

  public void cancel() {
    started = false;
    status = STATUS_CANCELED;
    rollback();
  }

}
