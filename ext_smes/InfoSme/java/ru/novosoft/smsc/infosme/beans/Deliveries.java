package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.Transliterator;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.infosme.backend.Task;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.util.jsp.MultipartServletRequest;
import ru.novosoft.util.jsp.MultipartDataSource;

import javax.servlet.http.HttpServletRequest;
import javax.sql.DataSource;
import java.io.*;
import java.util.List;
import java.util.Collection;
import java.util.ArrayList;
import java.sql.Connection;
import java.sql.SQLException;
import java.sql.PreparedStatement;
import java.sql.Statement;
import java.text.SimpleDateFormat;

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

    private String mbStat   = null;
    private String mbNext   = null;
    private String mbCancel = null;

    private int stage = 0;
    private File incomingFile = null;

    private boolean transliterate = false;
    private Task task = new Task();
    private String taskTableName = null;

    protected int init(List errors)
    {
      int result = super.init(errors);
      if (result != RESULT_OK)
        return result;

      resetTask();
      return RESULT_OK;
    }

    public int process(HttpServletRequest request)
    {
      int result = super.process(request);
      if (result != RESULT_OK)
        return result;

      if (mbNext != null) {
        mbNext = null;   return next();
      } else if (mbCancel != null) {
        mbCancel = null; return cleanup();
      } else if (mbStat != null) {
        mbStat = null;   return InfoSmeBean.RESULT_STAT;
      }

      return RESULT_OK;
    }

    private int next()
    {
        switch (stage)
        {
            case 0: stage++; return RESULT_OK;
            case 1: return RESULT_OK;
            case 2: return processFile();
            case 3: return processTask();
            case 4: return finishTask();
        }
        stage = 0;
        return RESULT_DONE;
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

        if (taskTableName != null) {
            cleanupTaskDB(); // Drop task table by task.id
            taskTableName = null;
        }

        return RESULT_DONE;
    }

    public int receiveFile(MultipartServletRequest multi)
    {
      if (multi == null)
        return error(SMSCErrors.error.services.fileNotAttached);

      MultipartDataSource dataFile = null;
      try
      {
        dataFile = multi.getMultipartDataSource(ABONENTS_FILE_PARAM);
        if (dataFile == null)
          return error(SMSCErrors.error.services.fileNotAttached);

        incomingFile = Functions.saveFileToTemp(dataFile.getInputStream(),
                     new File(WebAppFolders.getWorkFolder(), "INFO_SME_abonents.list"));
        incomingFile.deleteOnExit();
        logger.debug("File '"+incomingFile.toString()+"' received");
      }
      catch (Throwable t) {
        return error(SMSCErrors.error.services.couldntReceiveFile, t);
      }
      finally {
        if (dataFile != null) {
          dataFile.close();
          dataFile = null;
        }
      }

      if (errors.size() == 0) {
        stage++; return RESULT_OK;
      }
      else
        return RESULT_ERROR;
    }

    private Object countersSync = new Object();
    private long fileSize = 1;
    private long filePos  = 0;
    private long msgCount = 0;

    public int getProgress() {
        double fPos = 0;
        synchronized(countersSync) { fPos = filePos; }
        return ((int)((fPos/fileSize)*100))%101;
    }
    public long getMessages() {
        synchronized(countersSync) { return msgCount; }
    }
    private void resetCounters(long fs) {
        synchronized(countersSync) {
            msgCount = 0; filePos = 0; fileSize = fs;
        }
    }

    class GenerateThread extends Thread
    {
        private Connection connection = null;
        private BufferedInputStream bis = null;

        private boolean isStopping = false;
        private Object  shutdownSemaphore = new Object();

        public GenerateThread(FileInputStream fis, Connection connection) throws IOException
        {
            this.connection = connection;
            resetCounters(fis.getChannel().size());
            this.bis = new BufferedInputStream(fis, 1024*10); // 10Kb
        }

        private void incProgress(boolean fin) {
            synchronized(countersSync) {
                if (!fin) msgCount++;
                else filePos = fileSize+1;
            }
        }

        private final static int ESC_CR = 13;
        private final static int ESC_LF = 10;

        private String readLine() throws IOException
        {
            int ch = -1; StringBuffer sb = new StringBuffer(30);
            while (!isStopping)
            {
                if ((ch = this.bis.read()) == -1) return null;
                synchronized(countersSync) { filePos++; }
                if (ch == ESC_CR) continue;
                else if (ch == ESC_LF) break;
                else sb.append((char)ch);
            }
            return sb.toString().trim();
        }

        private void rollback()
        {
            try { if (connection != null) connection.rollback(); }
            catch (Throwable tt) { logger.error("Failed to rollback", tt); }
        }
        public void run()
        {
            final String taskId = task.getId();
            status = STATUS_OK; statusStr = STATUS_STR_GENERATING;
            String line = null; long uncommited = 0;
            try
            {
                generateThreadRunning = true;
                logger.debug("Starting messages generating for task '"+taskId+"'");

                // create & call messages generating statement
                PreparedStatement stmt = connection.prepareStatement(prepareInsertMessageSql());
                stmt.setByte  (1, (byte)0); // NEW_STATE
                stmt.setDate  (3, new java.sql.Date(System.currentTimeMillis()));
                stmt.setString(4, task.getText());

                while(!isStopping)
                {
                    line = readLine();
                    if (line == null || line.length() <= 0) {
                        incProgress(true); break;
                    }
                    Mask address = new Mask(line); // throws AdminException
                    stmt.setString(2, address.getMask());
                    stmt.executeUpdate();
                    incProgress(false);
                    if (++uncommited >= 50) { 
                        connection.commit(); uncommited = 0;
                    }
                }
                if (uncommited > 0) connection.commit();
                stmt.close();
                
                // create & call insert statistics statement
                stmt = connection.prepareStatement(INSERT_STAT_SQL);
                SimpleDateFormat formatter = new SimpleDateFormat("yyyyMMddHH");
                stmt.setInt   (1, Integer.parseInt(formatter.format(new java.util.Date())));
                stmt.setString(2, taskId);
                stmt.setLong  (3, getMessages());
                stmt.executeUpdate();
                connection.commit(); stmt.close();

                status = STATUS_DONE; statusStr = STATUS_STR_DONE;

            } catch (AdminException ae) {
              rollback(); logger.error("Incorrect abonent address format '"+line+"'.", ae);
              setError("Incorrect abonent address format '"+line+"'.");
            } catch (SQLException se) {
              rollback(); logger.error("DataSource error", se);
              setError("DataSource error. Details: "+se.getMessage());
            } catch (IOException ex) {
              rollback(); logger.error("File input error", ex);
              setError("File input error. Details: "+ex.getMessage());
            } catch (Throwable th) {
              rollback(); logger.error("Unexpected error occured", th);
              setError("Unexpected error occured. "+th.getMessage());
            } finally {
              try { if (connection != null) connection.close(); } 
              catch (SQLException e) { logger.error("Can't close connection to DB", e); }
              try { bis.close(); }
              catch (Throwable th) { logger.error("Can't close input stream", th); }
              synchronized(shutdownSemaphore) {
                shutdownSemaphore.notifyAll();
              }
              logger.debug("Messages generation for task '"+taskId+"' finished");
              generateThreadRunning = false; generateThread = null; connection = null;
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

    public final static int STATUS_OK = 0;
    public final static int STATUS_ERR = -1;
    public final static int STATUS_DONE = 1;

    private final static String STATUS_STR_GENERATING = "Generating messages...";
    private final static String STATUS_STR_INIT       = "Initialization...";
    private final static String STATUS_STR_DONE       = "Finished";
    private final static String STATUS_STR_ERR        = "Error - ";

    private int status = STATUS_OK;
    private String statusStr = STATUS_STR_INIT;

    public String getStatusStr() {
        return statusStr;
    }
    public int getStatus() {
        return status;
    }
    private void setError(String str) {
        status = STATUS_ERR; statusStr = STATUS_STR_ERR + str;
    }

    private final static String DROP_TABLE_SQL = "DROP TABLE ";
    private final static String CREATE_TABLE_SQL = "CREATE TABLE ";
    private final static String CREATE_TABLE_BODY_SQL =
        " (ID       NUMBER          NOT NULL,"+
        " STATE     NUMBER(3)       NOT NULL,"+
        " ABONENT   VARCHAR2(30)    NOT NULL,"+
        " SEND_DATE DATE            NOT NULL,"+
        " MESSAGE   VARCHAR2(2000)  NULL,"+
        " PRIMARY KEY (ID))";

    // "CREATE INDEX %s_SD_IDX ON %s (STATE, SEND_DATE)";
    // "CREATE INDEX %s_AB_IDX ON %s (STATE, ABONENT)";
    private final static String CREATE_INDEX_SQL = "CREATE INDEX ";
    private final static String CREATE_INDEX1_PX_SQL = "_SD_IDX ON ";
    private final static String CREATE_INDEX2_PX_SQL = "_AB_IDX ON ";
    private final static String CREATE_INDEX1_BODY_SQL = "(STATE, SEND_DATE)";
    private final static String CREATE_INDEX2_BODY_SQL = "(STATE, ABONENT)";
    private final static String INSERT_MSG_SQL = "INSERT INTO ";
    private final static String INSERT_MSG_BODY_SQL =
        " (ID, STATE, ABONENT, SEND_DATE, MESSAGE) VALUES (INFOSME_MESSAGES_SEQ.NEXTVAL, ?, ?, ?, ?)";
    private final static String INSERT_STAT_SQL = 
        "INSERT INTO INFOSME_TASKS_STAT (period, task_id, generated, delivered, retried, failed)"+
        " VALUES(?, ?, ?, 0, 0, 0)";
    private String prepareCreateTableSql() {
        return CREATE_TABLE_SQL +  taskTableName + CREATE_TABLE_BODY_SQL;
    }
    private String prepareCreateIndex1Sql() {
        return CREATE_INDEX_SQL + taskTableName +
               CREATE_INDEX1_PX_SQL + taskTableName + CREATE_INDEX1_BODY_SQL;
    }
    private String prepareCreateIndex2Sql() {
        return CREATE_INDEX_SQL + taskTableName +
               CREATE_INDEX2_PX_SQL + taskTableName + CREATE_INDEX2_BODY_SQL;
    }
    private String prepareDropTableSql() {
        return DROP_TABLE_SQL + taskTableName;
    }
    private String prepareInsertMessageSql() {
        return INSERT_MSG_SQL + taskTableName + INSERT_MSG_BODY_SQL;
    }

    private void cleanupTaskDB()
    {
        Connection connection = null;
        try {
            DataSource ds = getInfoSmeContext().getDataSource();
            if (ds == null || (connection = ds.getConnection()) == null)
                throw new SQLException("Failed to obtain DataSource connection");
            cleanupTaskDB(connection);
        } catch (SQLException exc) {
            try { if (connection != null) connection.close(); } catch (Exception e) {
              logger.error("Can't close connection to DB", e);
            }
        }
    }
    private void cleanupTaskDB(Connection connection)
    {
        logger.debug("DB cleanup called");
        if (connection == null || taskTableName == null) return;
        try {
          Statement stmt = connection.createStatement();
          stmt.execute(prepareDropTableSql());
          connection.commit();
        } catch(Exception e) {
          logger.error("Task cleanup failed", e);
        } finally {
          try { connection.close(); } catch (Exception e) {
            logger.error("Can't close connection to DB", e);
          }
        }
        logger.debug("DB cleanup called finished");
    }
    private int processFile()
    {
        if (generateThread != null || generateThreadRunning) return RESULT_DONE;
        if (!checkAndPrepareTask()) return RESULT_ERROR;

        Connection connection = null;
        FileInputStream fis = null;
        try
        {
          DataSource ds = getInfoSmeContext().getDataSource();
          if (ds == null || (connection = ds.getConnection()) == null)
              throw new SQLException("Failed to obtain DataSource connection");

          logger.debug("Creating DB tables for task '"+task.getId()+"'");
          Statement stmt = connection.createStatement();
          stmt.execute(prepareCreateTableSql());
          stmt.execute(prepareCreateIndex1Sql());
          stmt.execute(prepareCreateIndex2Sql());
          connection.commit();

          logger.debug("DB tables for task '"+task.getId()+"' created. Start generating...");
          fis = new FileInputStream(incomingFile);
          generateThread = new GenerateThread(fis, connection);
          status = STATUS_OK;
        }
        catch (SQLException exc) {
          cleanupTaskDB(connection);
          error("Failed to prepare service tables in DB. Details: ", exc.getMessage());
        }
        catch (Exception exc) {
          cleanupTaskDB(connection);
          try { if (fis != null) fis.close(); }
          catch (Exception e) { logger.error("Can't close input stream", e); }
          error("Abonents file incorrect. Details: ", exc.getMessage());
        }

        if (errors.size() == 0) {
            if (generateThread != null) generateThread.start();
            stage++; return RESULT_OK;
        }
        return RESULT_ERROR;
    }

    private int processTask()
    {
        errors.clear();

        final String taskId = task.getId();
        Config backup = null;
        try
        {
          final Config oldConfig = getInfoSmeContext().getConfig();
          backup = (Config) oldConfig.clone();
          task.storeToConfig(oldConfig);
          oldConfig.save();
          if (getInfoSme().getInfo().getStatus() == ServiceInfo.STATUS_RUNNING)
            getInfoSmeContext().getInfoSme().addTask(taskId);
        }
        catch (Exception e)
        {
          final String msg = "Could not add task '"+taskId+"'. Details: "+e.getMessage();
          logger.error(msg, e);
          error(msg);
          if (backup != null) {
              logger.warn("Restoring old config.");
              try { backup.save(); } catch(Throwable th) {
                  final String err = "Failed to restore backup config";
                  logger.error(err);
                  error(err);
              }
          }
          cleanupTaskDB();
        }

        stage++;
        if (errors.size() <= 0) {
            status = STATUS_OK; statusStr = STATUS_STR_INIT;
            return RESULT_OK;
        }
        setError("Failed to add task '"+taskId+"'");
        return RESULT_ERROR;
    }
    private void resetTask()
    {
        task.setDelivery(true);
        task.setProvider(Task.INFOSME_EXT_PROVIDER);
        task.setPriority(10);
        task.setMessagesCacheSize(100);
        task.setMessagesCacheSleep(1);
        task.setUncommitedInGeneration(10);
        task.setUncommitedInProcess(10);
        task.setEnabled(true);
        task.setKeepHistory(true);
        task.setReplaceMessage(false);
        task.setRetryOnFail(true);
        task.setRetryTime("01:00:00");
        task.setSvcType("dlvr");
        task.setActivePeriodStart("09:00:00");
        task.setActivePeriodEnd("21:00:00");
        task.setTransactionMode(true);
        task.setValidityPeriod("12:00:00");
    }
    private boolean checkAndPrepareTask()
    {
        errors.clear();

        String taskId = task.getId();
        if (taskId == null || (taskId = taskId.trim()).length() <= 0) {
            task.setId(""); error("Task id is undefined");
        }
        else task.setId(taskId);

        /*else if (taskId.length() > 7)
          error("Task id should be less than 8 charecters");
        else if (!taskId.matches("^(\\p{Alnum}){1,7}$"))
          error("Task id should be alpha-numeric");*/

        String taskName = task.getName();
        if (taskName == null || (taskName = taskName.trim()).length() <= 0) {
            task.setName(""); error("Task name is undefined");
        }
        else task.setName(taskName);
        if (task.getPriority() <= 0 || task.getPriority() > 100) {
            error("Task priority should be in [1, 100]");
        }
        String text = task.getText();
        if (text == null || (text = text.trim()).length() <= 0) {
            task.setText(""); error("Message text is empty");
        }
        else task.setText((transliterate) ? Transliterator.translit(text):text);

        if (task.isContainsInConfig(getConfig()))
            error("Task '"+taskId+"' already exists. Please specify another id");

        if (errors.size() > 0) return false;
        try {
          taskTableName = getTablesPrefix() + taskId;
        } catch (Exception exc) {
          error("Invalid settings. Details: "+exc.getMessage());
        }
        return (errors.size() <= 0);
    }
    private String getTablesPrefix() throws Exception
    {
        String tableNamesPrefix = getInfoSmeContext().getConfig().getString("InfoSme.tasksTablesPrefix");
        if (tableNamesPrefix == null)
          throw new Exception("Parameter 'InfoSme.tasksTablesPrefix' is undefined");
        return tableNamesPrefix;
    }
    private int finishTask()
    {
        errors.clear();
        resetTask(); stage = 0;
        status = STATUS_OK; statusStr = STATUS_STR_INIT;
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

    public boolean isTransactionMode() {
      return task.isTransactionMode();
    }
    public void setTransactionMode(boolean transactionMode) {
      this.task.setTransactionMode(transactionMode);
    }

    public String getEndDate() {
      return task.getEndDate();
    }
    public void setEndDate(String endDate) {
      this.task.setEndDate(endDate);
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
