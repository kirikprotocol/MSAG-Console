package ru.novosoft.smsc.infosme.backend.commands;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.infosme.backend.Task;
import ru.novosoft.smsc.infosme.backend.BlackListManager;
import ru.novosoft.smsc.util.Transliterator;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.config.Config;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * User: artem
 * Date: Aug 6, 2007
 */

public class TaskBuilder extends Thread {

  private final static int ESC_CR = 13;
  private final static int ESC_LF = 10;

  private static final SimpleDateFormat endDateFormat = new SimpleDateFormat("dd.MM.yyyy");
  private static final SimpleDateFormat activePeriodDateFormat = new SimpleDateFormat("hh:mm:ss");

  private String file;
  private Config oldConfig;
  private String processedFile;
  private final InfoSmeContext smeContext;

  private String taskId;
  private TaskProps taskProps;
  private Task task;

  public TaskBuilder(String file, InfoSmeContext smeContext) {
    this.smeContext = smeContext;
    this.file = file;
    initTask();
  }

  public TaskBuilder(String file, InfoSmeContext smeContext, TaskProps taskProps) {
    if(!taskProps.validateNull()) {
      throw new NullPointerException("Some arguments of TaskProps are null");
    }
    this.smeContext = smeContext;
    this.file = file;
    this.taskProps = taskProps;
    initTask();
  }

  private void initTask() {
    try {
      task = smeContext.getTaskManager().createTask();
      taskId = task.getId();
    } catch (AdminException e) {
      e.printStackTrace();
    }
  }

  private synchronized void removeTask(Task task) {
    try {
      oldConfig.save();
      try {
          smeContext.getInfoSme().removeTask(task.getId());
        } catch (Throwable e) {
          e.printStackTrace();
        }
//      smeContext.resetConfig();
//      if (task != null) {
//        task.removeFromConfig(smeContext.getConfig());
//        smeContext.getConfig().save();
//        smeContext.getInfoSme().removeTask(task.getId());
//      }

    } catch (Throwable e) {
    }
  }

  public void run() {
    if(taskId==null) {
      System.out.println("ERROR: Init task error, task is null");
      return;
    }
    System.out.println("Task builder started");
    final String fileName = new File(file).getName();


    int i = fileName.lastIndexOf('.');
    String taskName = (i >= 0) ? fileName.substring(0, i) : fileName;
    System.out.println("Task name=" + taskName);
//    task.setId(taskName);
    task.setName(taskName);

    System.out.println("Copy file: " + file);

    try {
      oldConfig = (Config)smeContext.getConfig().clone();
    } catch (CloneNotSupportedException e) {
      e.printStackTrace();
      return;
    }

    new File(file).renameTo(new File(file + ".processed"));
    processedFile = file + ".processed";

    InputStreamReader is = null;

    try {
      System.out.println("Create task...");
      resetTask(task, false);
      checkAndPrepareTask(task, smeContext, getFileCount(), false, false);

      task.setStatus("INPROGRESS");
      if(taskProps!=null) {
        String activeDayStr = taskProps.getDays();
        List activeDays = new LinkedList();
        Functions.addValuesToCollection(activeDays,activeDayStr,",",true);
        task.setActiveWeekDays(activeDays);
        task.setActiveWeekDaysSet(activeDays);

        task.setStartDate(taskProps.getDateBegin());
        task.setEndDate(taskProps.getDateEnd());
        task.setActivePeriodStart(taskProps.getTimeBegin());
        task.setActivePeriodEnd(taskProps.getTimeEnd());
        task.setAddress(taskProps.getAddress());
        task.setTransactionMode(taskProps.isTxmode());
      }
      is = new InputStreamReader(new FileInputStream(processedFile), Functions.getLocaleEncoding());

      int count = 0;

      storeTaskToConfig();
      System.out.println("Task generation....");
      smeContext.getInfoSme().addTask(task.getId());

      long currentTime = System.currentTimeMillis();
      Collection messages = getMessages(task, is, 1000, new Date(currentTime));
      while (messages != null && messages.size() > 0) {
        smeContext.getInfoSme().addDeliveryMessages(task.getId(), messages);
        count += messages.size();
        currentTime -= 1000;
        messages = getMessages(task, is, 1000,  new Date(currentTime));
      }

//      smeContext.getInfoSme().addStatisticRecord(task.getId(), new Date(), count, 0, 0, 0);
      smeContext.getInfoSme().endDeliveryMessageGeneration(task.getId());
      System.out.println("Task generation ok");

      task.setStatus("GENERATED");
      storeTaskToConfig();
    } catch (Exception e) {
      e.printStackTrace();
      removeTask(task);
      new File(processedFile).renameTo(new File(file + ".failed"));
      saveTaskFail(e);
    } finally {
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {
      }
    }
  }

  private void storeTaskToConfig() throws Exception{
    System.out.println("Store task to config...");
    task.storeToConfig(smeContext.getConfig());
    smeContext.getConfig().save();
  }

  private void saveTaskFail(Exception e) {
    PrintWriter out = null;

    try {
      out = new PrintWriter(new FileOutputStream(file + ".stacktrace"));
      e.printStackTrace(out);
    } catch (FileNotFoundException ex) {
      ex.printStackTrace();
    } finally {
      if (out != null)
        out.close();
    }
  }

  private long getFileCount() throws IOException {
    int fileCount = 0;

    InputStreamReader is = null;

    try {
      is = new InputStreamReader(new FileInputStream(processedFile), Functions.getLocaleEncoding());

      while (readLine(is) != null)
        fileCount++;

    } catch (IOException e) {
      throw new IOException(e.getMessage());
    } finally {
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {
      }
    }

    return fileCount;
  }

  private String readLine(InputStreamReader is) throws IOException {
    int ch = -1; StringBuffer sb = new StringBuffer(30);
    while (true)
    {
      if ((ch = is.read()) == -1) break;
      if (ch == ESC_CR) continue;
      else if (ch == ESC_LF) break;
      else sb.append((char)ch);
    }
    return (sb.length() == 0) ? null : sb.toString().trim();
  }

  public List getMessages(Task task, InputStreamReader is, int limit, Date sendDate) throws IOException, AdminException {
    final List list = new ArrayList();
    try {
      String line;
      BlackListManager blm = smeContext.getBlackListManager();
      for (int i=0; i < limit && ((line = readLine(is)) != null); i++) {

        StringTokenizer st = new StringTokenizer(line, "|");
        if (st.hasMoreTokens()) {
          final Message msg = new Message();
          String msisdn = st.nextToken();
          if (blm.contains(msisdn))
            continue;
          msg.setAbonent(msisdn);
          msg.setMessage(st.hasMoreTokens() ? st.nextToken() : task.getText());
          msg.setState(Message.State.NEW);
          msg.setSendDate(sendDate);
          list.add(msg);
        }
      }
    } catch (EOFException e) {
    }
    return list;
  }

  public void resetTask(Task task, boolean admin) {
    task.setDelivery(true);
    task.setProvider(Task.INFOSME_EXT_PROVIDER);
    task.setPriority(10);
    task.setMessagesCacheSize(2000);
    task.setMessagesCacheSleep(600);
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
    task.setValidityPeriod(admin ? "01:00:00" : "00:45:00");
  }

  public Collection checkAndPrepareTask(Task task, InfoSmeContext smeContext, long contentCount, boolean transliterate, boolean admin) throws AdminException {
    ArrayList errors = new ArrayList();
    String taskId = task.getId();
//    if (taskId == null || (taskId = taskId.trim()).length() <= 0) {
//      task.setId("");
//      errors.add("infosme.error.task_id_undefined");
//    } else
//      task.setId(taskId);

    String taskName = task.getName();
    if (taskName == null || (taskName = taskName.trim()).length() <= 0) {
      task.setName("");
      errors.add("infosme.error.task_name_undefined");
    } else
      task.setName(taskName);

    if (task.getPriority() <= 0 || task.getPriority() > 100)
      errors.add("infosme.error.task_priority_invalid");

    String text = task.getText();
    if (text == null || (text = text.trim()).length() <= 0) {
      task.setText("");
      errors.add("infosme.error.task_msg_text_empty");
    } else
      task.setText((transliterate) ? Transliterator.translit(text):text);

    if (task.isContainsInConfig(smeContext.getConfig()))
      errors.add("Task with id='"+taskId+"' already exists. Please specify another id");

    if (task.isContainsInConfigByName(smeContext.getConfig()))
      errors.add("Task with name='"+taskName+"' already exists. Please specify another name");

    if (!admin)
      calculateRetryOnFail(task, smeContext, contentCount);

    return errors;
  }

  private void calculateRetryOnFail(Task task, InfoSmeContext smeContext, long contentCount) throws AdminException {
    long retryOnFail = -1;
    if (task.getEndDate() == null || task.getEndDate().length() == 0)
      retryOnFail = 24*3600;
    else {
      try {
        final int maxSmsSec = smeContext.getConfig().getInt("InfoSme.maxMessagesPerSecond") / 2;
        final long totalTime = calculateTotalTime(task) / 1000 ; // total time in seconds

        long trySeconds = contentCount / maxSmsSec ; // total work time in seconds
        if (trySeconds == 0)
          trySeconds = 1; // One second will be enough

        long ntries = totalTime / trySeconds; // Count of tries

        if (ntries - 1 > 0)
          retryOnFail = ntries * trySeconds;

      } catch (ParseException e) {
        throw new AdminException(e.getMessage());
      } catch (Config.WrongParamTypeException e) {
        throw new AdminException(e.getMessage());
      } catch (Config.ParamNotFoundException e) {
        throw new AdminException(e.getMessage());
      }
    }

    if (retryOnFail > 0) {
      task.setRetryOnFail(true);
      long hours = Math.min(retryOnFail/3600, 24);
      long minutes = hours == 24 ? 0 : (retryOnFail - hours*3600) / 60;
      long seconds = hours == 24 ? 0 : retryOnFail - hours*3600 - minutes*60;
      task.setRetryTime(getDoubleCharNumber(hours) + ":" + getDoubleCharNumber(minutes) + ":" + getDoubleCharNumber(seconds));
    } else
      task.setRetryOnFail(false);
  }

  private String getDoubleCharNumber(long num) {
    String res = String.valueOf(num);
    if (res.length() == 1)
      res = "0" + res;
    return res;
  }

  private long calculateTotalTime(Task task) throws ParseException {
    long result = 0;
    String[] days = new String[] {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    final long endDate = endDateFormat.parse(task.getEndDate()).getTime();

    final Calendar start = Calendar.getInstance();
    final Calendar end = Calendar.getInstance();

    start.setTimeInMillis(activePeriodDateFormat.parse(task.getActivePeriodStart()).getTime());
    end.setTimeInMillis(activePeriodDateFormat.parse(task.getActivePeriodEnd()).getTime());

    final long dayCount = end.getTimeInMillis() - start.getTimeInMillis();

    final Calendar cal = Calendar.getInstance();

    long date = System.currentTimeMillis();
    cal.setTimeInMillis(date);

    start.set(Calendar.DAY_OF_MONTH, cal.get(Calendar.DAY_OF_MONTH));
    start.set(Calendar.MONTH, cal.get(Calendar.MONTH));
    start.set(Calendar.YEAR, cal.get(Calendar.YEAR));

    end.set(Calendar.DAY_OF_MONTH, cal.get(Calendar.DAY_OF_MONTH));
    end.set(Calendar.MONTH, cal.get(Calendar.MONTH));
    end.set(Calendar.YEAR, cal.get(Calendar.YEAR));

    if (cal.getTime().after(start.getTime())) {
      if (task.isWeekDayActive(days[cal.get(Calendar.DAY_OF_WEEK) - 1]))
        result += (end.getTimeInMillis() - cal.getTimeInMillis());
      date += 1000 * 60 * 60 * 24;
    }

    for (; date < endDate; date += 1000 * 60 * 60 * 24) {
      cal.setTimeInMillis(date);
      if (task.isWeekDayActive(days[cal.get(Calendar.DAY_OF_WEEK) - 1]))
        result += dayCount;
    }

    return result;
  }

  public String getTaskId() {
    return taskId; 
  }
}
