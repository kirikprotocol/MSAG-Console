package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.infosme.backend.Task;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.Transliterator;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.util.jsp.MultipartDataSource;
import ru.novosoft.util.jsp.MultipartServletRequest;

import javax.servlet.http.HttpServletRequest;
import java.io.*;
import java.util.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 05.09.2005
 * Time: 13:54:02
 * To change this template use File | Settings | File Templates.
 */
public class Deliveries extends InfoSmeBean
{
    public final static String ABONENTS_FILE_PARAM = "abonentsFile";

    private final static int ESC_CR = 13;
    private final static int ESC_LF = 10;

    public final static int STATUS_OK = 0;
    public final static int STATUS_ERR = -1;
    public final static int STATUS_DONE = 1;

    private final static String STATUS_STR_GENERATING = "Generating messages...";
    private final static String STATUS_STR_INIT       = "Initialization...";
    private final static String STATUS_STR_DONE       = "Finished";
    private final static String STATUS_STR_ERR        = "Error - ";

    private static final SimpleDateFormat endDateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");
    private static final SimpleDateFormat activePeriodDateFormat = new SimpleDateFormat("hh:mm:ss");

    private int status = STATUS_OK;
    private String statusStr = STATUS_STR_INIT;


    private String mbDlstat = null;
    private String mbStat   = null;
    private String mbNext   = null;
    private String mbCancel = null;

    private int stage = 0;
    private int fileCount = -1;
    private File incomingFile = null;

    private boolean transliterate = false;
    private boolean retryOnFail = true;

//    private Config backup = null;

    private Task task = new Task();

    protected int init(List errors)
    {
      int result = super.init(errors);
      if (result != RESULT_OK)
        return result;

//      try {
//        if (backup == null) {
//          final Config oldConfig = getInfoSmeContext().getConfig();
//          backup = (Config) oldConfig.clone();
//        }
//
//      } catch (Exception e) {
//        logger.error("Can't init InfoSme", e);
//        return RESULT_ERROR;
//      }

      return RESULT_OK;
    }

    public boolean isUserAdmin(HttpServletRequest request) {
      return request.isUserInRole("infosme-admin");
    }

    public int process(HttpServletRequest request)
    {
      int result = super.process(request);
      if (result != RESULT_OK)
        return result;

      if (mbNext != null) {
        mbNext = null;   return next(request);
      } else if (mbCancel != null) {
        mbCancel = null; return cleanup();
      } else if (mbStat != null) {
        mbStat = null;   return InfoSmeBean.RESULT_STAT;
      } else if (mbDlstat != null) {
        mbDlstat = null;   return InfoSmeBean.RESULT_DLSTAT;
      }

      return RESULT_OK;
    }

    private int next(HttpServletRequest request)
    {
        switch (stage)
        {
            case 0: initCleanup(request); stage++; return RESULT_OK;
            case 1: return RESULT_OK;
            case 2: return processFile(request);
            case 3: return processTask();
            case 4: return finishTask(request);
        }
        stage = 0;
        fileCount = 0;
        return RESULT_DONE;
    }

    private void initCleanup(HttpServletRequest request)
    {
      stage = 0;
      fileCount = 0;

      if (incomingFile != null && incomingFile.isFile() &&
          incomingFile.exists()) incomingFile.delete();
      incomingFile = null;

      transliterate = false;
      task = new Task();
      resetTask(request);
    }

    private int cleanup()
    {
        if (incomingFile != null && incomingFile.isFile() &&
            incomingFile.exists()) incomingFile.delete();

        incomingFile = null;
        mbNext = null; mbCancel = null; mbStat = null; stage = 0;
        status = STATUS_OK; statusStr = STATUS_STR_INIT;

        if (generateThread != null) {
            generateThread.shutdown();
            generateThread = null;
        }

        removeTask();

        return RESULT_DONE;
    }

    public int receiveFile(MultipartServletRequest multi)
    {
      if (multi == null)
        return error("infosme.error.file_not_attached");

      MultipartDataSource dataFile = null;
      try
      {
        dataFile = multi.getMultipartDataSource(ABONENTS_FILE_PARAM);
        if (dataFile == null)
          return error("infosme.error.file_not_attached");

//        incomingFile = Functions.saveFileToTemp(dataFile.getInputStream(),
//                     new File(WebAppFolders.getWorkFolder(), "INFO_SME_abonents.list"));
        incomingFile = saveFileToTemp(dataFile.getInputStream(),
                     new File(WebAppFolders.getWorkFolder(), "INFO_SME_abonents.list"));
        incomingFile.deleteOnExit();
        logger.debug("File '"+incomingFile.toString()+"' received. " + fileCount + " messages");
      }
      catch (Throwable t) {
        return error("infosme.error.file_failed_receive", t);
      }
      finally {
        if (dataFile != null) {
          dataFile.close();
          dataFile = null;
        }
      }

      if (errors.size() == 0) {
        stage++;
        return RESULT_OK;
      } else
        return RESULT_ERROR;
    }

    private File saveFileToTemp(InputStream in, File file) throws IOException {
      File tmpFile = Functions.createNewFilenameForSave(file);

      OutputStream out = null;
      String str;
      fileCount = 0;

      try {
        out = new BufferedOutputStream(new FileOutputStream(tmpFile));

        while ((str = readLine(in)) != null) {
          out.write(str.getBytes());
          out.write(ESC_LF);
          fileCount++;
        }

        out.flush();
      } catch (IOException e) {
        throw new IOException(e.getMessage());
      } finally {
        try {
          if (out != null)
            out.close();
        } catch (IOException e) {
        }
      }

      in.close();

      return tmpFile;
    }

    private String readLine(InputStream bis) throws IOException {
      int ch = -1;
      final StringBuffer sb = new StringBuffer(30);
      while (true) {
        if ((ch = bis.read()) == -1) break;
        if (ch == ESC_CR) continue;
        else if (ch == ESC_LF) break;
        else sb.append((char)ch);
      }

      return (sb.length() == 0) ? null : sb.toString().trim();
    }

    private Object countersSync = new Object();
    private boolean finalized = false;
    private long fileSize = 1;
    private long filePos  = 0;
    private long msgCount = 0;

    public int getProgress() {
        if (finalized) return 100;
        double fPos = 0;
        synchronized(countersSync) { fPos = filePos; }
        return ((int)((fPos/fileSize)*100))%101;
    }
    public long getMessages() {
        synchronized(countersSync) { return msgCount; }
    }
    private void resetCounters(long fs) {
        synchronized(countersSync) {
            msgCount = 0; filePos = 0; fileSize = fs; finalized = false;
        }
    }

    class GenerateThread extends Thread
    {
        private BufferedInputStream bis = null;

        private boolean isStopping = false;
        private Object  shutdownSemaphore = new Object();


        public GenerateThread(FileInputStream fis) throws IOException
        {
            resetCounters(fis.getChannel().size());
            this.bis = new BufferedInputStream(fis, 1024*10); // 10Kb
        }

        private void incProgress(boolean fin) {
            synchronized(countersSync) {
                if (!fin) msgCount++;
                else finalized = true;
            }
        }



        private String readLine() throws IOException
        {
            int ch = -1; StringBuffer sb = new StringBuffer(30);
            while (!isStopping)
            {
                if ((ch = this.bis.read()) == -1) break;
                synchronized(countersSync) { filePos++; }
                if (ch == ESC_CR) continue;
                else if (ch == ESC_LF) break;
                else sb.append((char)ch);
            }
            return (sb.length() == 0) ? null : sb.toString().trim();
        }

        private void rollback() {
          removeTask();
        }

        public void run()
        {
            final String taskId = task.getId();
            status = STATUS_OK; statusStr = STATUS_STR_GENERATING;
            String line = null;

            final Config oldConfig = getInfoSmeContext().getConfig();

            try
            {
                generateThreadRunning = true;
                logger.debug("Starting messages generating for task '"+taskId+"'");

                java.util.Date currentTime = (task.getStartDate() == null || task.getStartDate().trim().length() == 0)? new java.util.Date() : endDateFormat.parse(task.getStartDate());
                currentTime = new java.util.Date(currentTime.getTime() + Math.round(fileCount/1000) * 1000);
              
                task.storeToConfig(oldConfig);
                oldConfig.save();

                getInfoSmeContext().getInfoSme().addTask(taskId);

                final List messages = new ArrayList();

                while(!isStopping)
                {
                    line = readLine();
                    if (line == null || line.length() <= 0) {
                        incProgress(true); break;
                    }
                    Mask address = new Mask(line); // throws AdminException
                    final Message message = new Message();
                    message.setState(Message.State.NEW);
                    message.setSendDate(currentTime);
                    message.setMessage(task.getText());
                    message.setAbonent(address.getMask());

                    messages.add(message);

                    if (messages.size() > 1000) {
                      getInfoSmeContext().getInfoSme().addDeliveryMessages(task.getId(), messages);
                      messages.clear();
                      currentTime = new Date(currentTime.getTime() - 1000);
                    }

                    incProgress(false);
                }

              if (messages.size() > 0) {
                getInfoSmeContext().getInfoSme().addDeliveryMessages(task.getId(), messages);
                messages.clear();
              }

                getInfoSmeContext().getInfoSme().addStatisticRecord(task.getId(), new Date(), (int)getMessages(), 0, 0 ,0);

                statusStr = STATUS_STR_DONE;
                status = STATUS_DONE;

//                stage++;
            } catch (AdminException ae) {
              logger.error("Error: " + ae.getMessage(), ae);
              rollback();
              setError("Error: " + ae.getMessage());
            } catch (IOException ex) {
              logger.error("File input error", ex);
              rollback();
              setError("File input error. Details: "+ex.getMessage());
            } catch (Throwable th) {
              logger.error("Unexpected error occured", th);
              rollback();
              setError("Unexpected error occured. "+th.getMessage());
            } finally {
              try { bis.close(); }
              catch (Throwable th) { logger.error("Can't close input stream", th); }
              synchronized(shutdownSemaphore) {
                shutdownSemaphore.notifyAll();
              }
              logger.debug("Messages generation for task '"+taskId+"' finished");
              generateThreadRunning = false; generateThread = null; //connection = null;
            }
        }
        public void shutdown()
        {
            logger.debug( "Shutdown called");
            synchronized(shutdownSemaphore) {
                isStopping = true;
                try {
                  bis.close();
                } catch (Exception e) {
                  logger.error("Can't close input stream", e);
                } catch(Throwable th) {
                  logger.error("Unexpected error occured", th);
                } finally {
                    try { shutdownSemaphore.wait(); } catch (InterruptedException e) {}
                }
            }
        }
    }

    GenerateThread generateThread = null;
    boolean generateThreadRunning = false;
    public boolean isInGeneration() {
        return generateThreadRunning;
    }


    public String getStatusStr() {
        return statusStr;
    }

    public int getStatus() {
        return status;
    }

    private void setError(String str) {
        status = STATUS_ERR; statusStr = STATUS_STR_ERR + str;
    }

    private synchronized void removeTask() {

      try {
//        if (backup != null) {
//          logger.warn("Restoring old config.");
//          backup.save();
//          backup = null;
//          getInfoSmeContext().resetConfig();
//          getInfoSmeContext().getConfig().save();
//        }

        getInfoSmeContext().resetConfig();
        if (task != null) {
          task.removeFromConfig(getInfoSmeContext().getConfig());
          getInfoSmeContext().getConfig().save();
          getInfoSmeContext().getInfoSme().removeTask(task.getId());
        }
      } catch (Throwable e) {
        logger.error("Failed rollback task");
        error("infosme.error.config_restore");
      }
    }

    private long calculateTotalTime() throws ParseException {
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

    private String getDoubleCharNumber(long num) {
      String res = String.valueOf(num);
      if (res.length() == 1)
        res = "0" + res;
      return res;
    }

    private void calculateRetryOnFail() {
      long retryOnFail = -1;
      if (task.getEndDate() == null || task.getEndDate().length() == 0)
        retryOnFail = 24*3600;
      else {
        try {
          final int maxSmsSec = getInfoSmeContext().getConfig().getInt("InfoSme.maxMessagesPerSecond") / 2;
          final long totalTime = calculateTotalTime() / 1000 ; // total time in seconds

          long trySeconds = fileCount / maxSmsSec ; // total work time in seconds
          if (trySeconds == 0)
            trySeconds = 1; // One second will be enough

          long ntries = totalTime / trySeconds; // Count of tries

          if (ntries - 1 > 0)
            retryOnFail = ntries * trySeconds;

        } catch (ParseException e) {
          logger.error(e,e);
        } catch (Config.WrongParamTypeException e) {
          logger.error(e,e);
        } catch (Config.ParamNotFoundException e) {
          logger.error(e,e);
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

    private int processFile(HttpServletRequest request) {
        if (generateThread != null || generateThreadRunning) return RESULT_DONE;
        if (!checkAndPrepareTask(request)) return RESULT_ERROR;

        FileInputStream fis = null;
        try
        {
          fis = new FileInputStream(incomingFile);
          generateThread = new GenerateThread(fis);

        }
        catch (Exception exc) {
          logger.error(exc);
          removeTask();
          try { if (fis != null) fis.close(); }
          catch (Exception e) { logger.error("Can't close input stream", e); }
          error("infosme.error.abonents_file_incorrect", exc.getMessage());
        }

        status = STATUS_OK;
        if (errors.size() == 0) {
            if (generateThread != null) generateThread.start();
            stage++;
            return RESULT_OK;
        }

        return RESULT_ERROR;
    }

    private int processTask()
    {
        stage++;

        if (status != STATUS_ERR) {
//          statusStr = STATUS_STR_INIT;

          return RESULT_OK;
        }

        return RESULT_ERROR;
    }
    private void resetTask(HttpServletRequest request)
    {
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
        task.setValidityPeriod(isUserAdmin(request) ? "01:00:00" : "00:45:00");
    }

    private boolean checkAndPrepareTask(HttpServletRequest request) {
        errors.clear();

        String taskId = task.getId();
        if (taskId == null || (taskId = taskId.trim()).length() <= 0) {
            task.setId(""); error("infosme.error.task_id_undefined");
        }
        else task.setId(taskId);

        String taskName = task.getName();
        if (taskName == null || (taskName = taskName.trim()).length() <= 0) {
            task.setName(""); error("infosme.error.task_name_undefined");
        }
        else task.setName(taskName);
        if (task.getPriority() <= 0 || task.getPriority() > 100) {
            error("infosme.error.task_priority_invalid");
        }
        String text = task.getText();
        if (text == null || (text = text.trim()).length() <= 0) {
            task.setText(""); error("infosme.error.task_msg_text_empty");
        }
        else task.setText((transliterate) ? Transliterator.translit(text):text);

        if (task.isContainsInConfig(getConfig()))
            error("Task with id='"+taskId+"' already exists. Please specify another id");
        if (task.isContainsInConfigByName(getConfig()))
            error("Task with name='"+taskName+"' already exists. Please specify another name");

      try {
        if (task.getStartDate() != null && task.getStartDate().trim().length() > 0)
          endDateFormat.parse(task.getStartDate());
      } catch (ParseException e) {
        error("Invalid task start date");
      }

      try {
        if (task.getEndDate() != null && task.getEndDate().trim().length() > 0)
          endDateFormat.parse(task.getEndDate());
      } catch (ParseException e) {
        error("Invalid task end date");
      }

      if (!isUserAdmin(request))
          calculateRetryOnFail();

        if (errors.size() > 0) return false;
        return (errors.size() <= 0);
    }

    private int finishTask(HttpServletRequest request)
    {
      if (status == STATUS_DONE) {
        try {
          getInfoSmeContext().getInfoSme().endDeliveryMessageGeneration(task.getId());
          getConfig().setBool(TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(task.getId()) + ".messagesHaveLoaded", true);
          getConfig().save();
//          backup = null;
        } catch (AdminException e) {
          logger.error(e);
        } catch (Config.WrongParamTypeException e) {
          logger.error(e);
        } catch (Exception e) {
          logger.error(e);
        }
      }
      errors.clear();
      resetTask(request); stage = 0; fileCount = 0;
      status = STATUS_OK; statusStr = STATUS_STR_INIT;
      if (incomingFile != null && incomingFile.isFile() && incomingFile.exists()) {
        incomingFile.delete(); incomingFile = null;
      }
      return RESULT_DONE;
    }

    /* ############################## Task accessors ############################## */

    public String getId() {
      return task.getId();
    }
    public void setId(String sectionName) {
      this.task.setId(sectionName);
    }

    public String getName() {
      return task.getName();
    }
    public void setName(String name) {
      this.task.setName(name);
    }

    public boolean isTransliterate() {
      return transliterate;
    }
    public void setTransliterate(boolean transliterate) {
      this.transliterate = transliterate;
    }

    public boolean isRetryOnFail() {
      return retryOnFail;
    }
    public void setRetryOnFail(boolean retryOnFail) {
      this.task.setRetryOnFail(retryOnFail);
    }

    public String getRetryTime() {
      return task.getRetryTime();
    }
    public void setRetryTime(String retryTime) {
      task.setRetryTime(retryTime);
    }

    public String getEndDate() {
      return task.getEndDate();
    }
    public void setEndDate(String endDate) {
      this.task.setEndDate(endDate);
    }

    public String getStartDate() {
      return task.getStartDate();
    }

    public void setStartDate(String startDate) {
      this.task.setStartDate(startDate);
    }


    public String getValidityPeriod() {
      return task.getValidityPeriod();
    }
    public void setValidityPeriod(String validityPeriod) {
      this.task.setValidityPeriod(validityPeriod);
    }

    public String getValidityDate() {
      return task.getValidityDate();
    }
    public void setValidityDate(String validityDate) {
      this.task.setValidityDate(validityDate);
    }

    public String getActivePeriodStart() {
      return task.getActivePeriodStart();
    }
    public void setActivePeriodStart(String activePeriodStart) {
      this.task.setActivePeriodStart(activePeriodStart);
    }

    public String getActivePeriodEnd() {
      return task.getActivePeriodEnd();
    }
    public void setActivePeriodEnd(String activePeriodEnd) {
      this.task.setActivePeriodEnd(activePeriodEnd);
    }

    public String[] getActiveWeekDays() {
      return (String[])(this.task.getActiveWeekDays().toArray());
    }
    public void setActiveWeekDays(String[] activeWeekDays) {
      Collection awd = new ArrayList();
      for(int i=0; i<activeWeekDays.length; i++) awd.add(activeWeekDays[i]);
      this.task.setActiveWeekDays(awd);
    }
    public boolean isWeekDayActive(String weekday) {
      return this.task.isWeekDayActive(weekday);
    }
    public String getActiveWeekDaysString()
    {
      String str = "";
      Collection awd = this.task.getActiveWeekDays();
      int total = (awd == null) ? 0:awd.size();
      if (total > 0) {
        int added=0;
        if (task.isWeekDayActive("Mon")) { str += "Monday";    if (++added < total) str += ", "; }
        if (task.isWeekDayActive("Tue")) { str += "Tuesday";   if (++added < total) str += ", "; }
        if (task.isWeekDayActive("Wed")) { str += "Wednesday"; if (++added < total) str += ", "; }
        if (task.isWeekDayActive("Thu")) { str += "Thursday";  if (++added < total) str += ", "; }
        if (task.isWeekDayActive("Fri")) { str += "Friday";    if (++added < total) str += ", "; }
        if (task.isWeekDayActive("Sat")) { str += "Saturday";  if (++added < total) str += ", "; }
        if (task.isWeekDayActive("Sun"))   str += "Sunday";
      }
      return str;
    }

    public String getText() {
      return task.getText();
    }
    public void setText(String text) {
      this.task.setText(text);
    }

    /* ############################## Bean properties ############################## */

    public int getStage() {
        return stage;
    }
    public void setStage(int stage) {
        this.stage = stage;
    }
    public String getMbStat() {
        return mbStat;
    }
    public void setMbStat(String mbStat) {
        this.mbStat = mbStat;
    }
    public String getMbDlstat() {
        return mbDlstat;
    }
    public void setMbDlstat(String mbDlstat) {
        this.mbDlstat = mbDlstat;
    }
    public String getMbNext() {
        return mbNext;
    }
    public void setMbNext(String mbNext) {
        this.mbNext = mbNext;
    }
    public String getMbCancel() {
        return mbCancel;
    }
    public void setMbCancel(String mbCancel) {
        this.mbCancel = mbCancel;
    }
}
