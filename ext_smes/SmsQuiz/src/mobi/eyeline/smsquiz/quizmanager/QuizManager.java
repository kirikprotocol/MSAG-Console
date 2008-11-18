package mobi.eyeline.smsquiz.quizmanager;

import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.jmx.log4j.LoggingMBean;
import mobi.eyeline.smsquiz.distribution.Distribution;
import mobi.eyeline.smsquiz.distribution.DistributionException;
import mobi.eyeline.smsquiz.distribution.DistributionManager;
import mobi.eyeline.smsquiz.quizmanager.dirlistener.DirListener;
import mobi.eyeline.smsquiz.quizmanager.dirlistener.Notification;
import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;
import mobi.eyeline.smsquiz.quizmanager.quiz.QuizBuilder;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyStatsDataSource;
import mobi.eyeline.smsquiz.subscription.SubManagerException;
import mobi.eyeline.smsquiz.subscription.SubscriptionManager;
import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

import javax.management.MBeanServer;
import javax.management.ObjectName;
import java.io.*;
import java.lang.management.ManagementFactory;
import java.util.Observable;
import java.util.Observer;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ThreadFactory;

/**
 * author: alkhal
 */
public class QuizManager implements Observer {

  private static final Logger logger = Logger.getLogger(QuizManager.class);
  private static QuizManager quizManager;


  private DirListener dirListener;
  private QuizCollector quizCollector;
  private static ReplyStatsDataSource replyStatsDataSource;
  private static DistributionManager distributionManager;
  private static SubscriptionManager subscriptionManager;        //init ConnectionPoll for DB needed

  private ScheduledExecutorService scheduledDirListener;
  private ScheduledExecutorService scheduledQuizCollector;
  private QuizBuilder quizBuilder;

  private QuizManagerMBean monitor;

  private ConcurrentHashMap<String, Quiz> quizesMap;
  private String quizDir;
  private String statusDir;
  private long listenerDelayFirst;
  private long listenerPeriod;
  private long collectorDelayFirst;
  private long collectorPeriod;
  private String dirResult;
  private String dirWork;

  public static void init(final String configFile, DistributionManager distributionManager, ReplyStatsDataSource replyStatsDataSource,
                          SubscriptionManager subscriptionManager) throws QuizException {
    quizManager = new QuizManager(configFile, distributionManager, replyStatsDataSource, subscriptionManager);
  }

  public static QuizManager getInstance() {
    return quizManager;
  }

  private QuizManager(final String configFile, DistributionManager dManager,
                      ReplyStatsDataSource rSource,
                      SubscriptionManager sManager) throws QuizException {
    if ((configFile == null) || (rSource == null) || (sManager == null)
        || (dManager == null)) {
      logger.error("Some arguments are null");
      throw new QuizException("Some arguments are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    distributionManager = dManager;
    replyStatsDataSource = rSource;
    subscriptionManager = sManager;
    String datePattern;
    String timePattern;
    try {
      final XmlConfig c = new XmlConfig();
      c.load(new File(configFile));
      PropertiesConfig config = new PropertiesConfig(c.getSection("quizmanager").toProperties("."));
      listenerDelayFirst = config.getLong("listener.delay.first", 30);
      listenerPeriod = config.getLong("listener.period.repeat", 30);
      collectorDelayFirst = config.getLong("collector.delay.first", 30);
      collectorPeriod = config.getLong("collector.period.repeat", 30);
      quizDir = config.getString("dir.quiz");
      statusDir = config.getString("dir.status");
      datePattern = "dd.MM.yyyy HH:mm";
      timePattern = "HH:mm";
      dirResult = config.getString("dir.result", "quizResults");
      dirWork = config.getString("dir.work", "quizManager_ab_mod");

      File file = new File(dirWork);
      if (!file.exists()) {
        file.mkdirs();
      }

    } catch (ConfigException e) {
      logger.error("Unable to construct QuizManager", e);
      throw new QuizException("Unable to construct QuizManager", e);
    }

    try {
      dirListener = new DirListener(quizDir);
    } catch (Exception e) {
      logger.error("Unable to construct DirListener", e);
      throw new QuizException("Unable to construct DirListener", e);
    }

    dirListener.addObserver(this);
    scheduledDirListener = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "QuizDirectorylistener");
      }
    });
    scheduledQuizCollector = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "QuizCollector");
      }
    });
    quizBuilder = new QuizBuilder(datePattern, timePattern);
    quizesMap = new ConcurrentHashMap<String, Quiz>();
    quizCollector = new QuizCollector(quizesMap, dirListener);

    File file = new File(statusDir);
    if (!file.exists()) {
      file.mkdirs();
    }
    monitor = new QuizManagerMBean(this);
  }

  public void start() {
    scheduledDirListener.scheduleAtFixedRate(dirListener, listenerDelayFirst, listenerPeriod, java.util.concurrent.TimeUnit.SECONDS);
    scheduledQuizCollector.scheduleAtFixedRate(quizCollector, collectorDelayFirst, collectorPeriod, java.util.concurrent.TimeUnit.SECONDS);
  }

  public void stop() {
    if (scheduledDirListener != null) {
      scheduledDirListener.shutdown();
    }
    if (scheduledQuizCollector != null) {
      scheduledQuizCollector.shutdown();
    }
    if (replyStatsDataSource != null) {
      replyStatsDataSource.shutdown();
    }
    if (subscriptionManager != null) {
      subscriptionManager.shutdown();
    }
    if (distributionManager != null) {
      distributionManager.shutdown();
    }
  }

  public Result handleSms(String address, String oa, String text) throws QuizException {
    if (logger.isInfoEnabled()) {
      logger.info("Manager handle sms:" + address + " " + oa + " " + text);
    }
    Quiz quiz = quizesMap.get(address);
    if ((quiz != null)&&(quiz.isActive())) {
      return quiz.handleSms(oa, text);
    }
    logger.warn("Quiz not found for address: " + address);
    if (logger.isInfoEnabled()) {
      logger.info("Available addresses: " + quizesMap.keySet().toString());
    }
    return null;
  }

  public void update(Observable o, Object arg) {
    logger.info("Updating quizfiles list...");
    Notification notification = (Notification) arg;
    if (notification.getStatus().equals(Notification.FileStatus.MODIFIED)) {

      try {
        modifyQuiz(notification);
      } catch (Exception e) {
        logger.error("Unable to modify quiz: " + notification.getFileName());
      }

    } else if (notification.getStatus().equals(Notification.FileStatus.CREATED)) {

      try {
        createQuiz(notification);
      } catch (Exception e) {
        logger.error("Unable to update quize: " + notification.getFileName());
      }
    } else if(notification.getStatus().equals(Notification.FileStatus.DELETED)) {

      try{
        deleteQuiz(notification);
      }
      catch (Exception e) {
        logger.error("Unable to delete quize: " + notification.getFileName());
      }
    }
    logger.info("Updating finished");
  }

  @SuppressWarnings({"ThrowableInstanceNeverThrown"})
  private void deleteQuiz(Notification notification) throws QuizException{
    String fileName = notification.getFileName();
    try {
      Quiz quiz = null;
      for (Map.Entry<String,Quiz> e : quizesMap.entrySet()) {
        if (e.getValue().getFileName().equals(fileName)) {
          if(!e.getValue().isActive()) {
            quizesMap.remove(e.getKey());
          }
          else {
            writeError(fileName, new Exception("Deleted quiz was active. see it's results"));
            e.getValue().exportStats();
          }
          return;
        }
      }
    }
    catch (Exception e) {
      writeError(notification.getFileName(), e);
      throw new QuizException(e);
    }

  }

  private void modifyQuiz(Notification notification) throws QuizException {

    String fileName = notification.getFileName();
    try {
      Quiz quiz = null;
      for (Quiz q : quizesMap.values()) {
        if (q.getFileName().equals(fileName)) {
          quiz = q;
        }
      }
      if (quiz == null) {
        logger.error("Can't find quiz into map with key: " + notification.getFileName());
        throw new QuizException("Can't find quiz into map with key: " + notification.getFileName());
      }

      quizBuilder.buildQuiz(fileName, null, quiz);
    } catch (QuizException e) {
      writeError(notification.getFileName(), e);
      throw e;
    }
    if (logger.isInfoEnabled()) {
      logger.info("Quiz modified: " + fileName);
    }
  }

  private void createQuiz(Notification notification) throws QuizException {
    if (logger.isInfoEnabled()) {
      logger.info("Create quiz...");
    }
    try {
      String fileName = notification.getFileName();
      Distribution distribution;
      File file = new File(fileName);

      final Quiz quiz = new Quiz(statusDir, file, replyStatsDataSource, distributionManager, dirResult, dirWork);
      distribution = new Distribution();
      quizBuilder.buildQuiz(fileName, distribution, quiz);
      QuizCreator quizCreator = new QuizCreator(quiz, distribution);
      if(quiz.isActive()) {
        quizCreator.start();
      } else {
        long delay = quiz.getDateBegin().getTime() -System.currentTimeMillis();
        ScheduledExecutorService executor = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
          public Thread newThread(Runnable r) {
            return new Thread(r,"QuizCreator: "+quiz.getFileName());
          }
        });
        executor.schedule(quizCreator,delay,java.util.concurrent.TimeUnit.MILLISECONDS);
      }

    }
    catch (Exception e) {
      writeError(notification.getFileName(), e);
      throw new QuizException(e);
    }
  }

  private void makeSubscribedOnly(Distribution distribution, String question, String quizId) throws QuizException {
    if ((distribution == null) || (distribution.getFilePath() == null) || (question == null)) {
      return;
    }
    File file = new File(distribution.getFilePath());
    if (!file.exists()) {
      logger.error("Distributions abonents file doesn't exist");
      throw new QuizException("Distributions abonents file doesn't exist", QuizException.ErrorCode.ERROR_INIT);
    }
    question = question.replace(System.getProperty("line.separator"), "\\n");
    String line;
    PrintWriter writer = null;
    BufferedReader reader = null;
    String modifiedFileName = dirWork + File.separator + quizId + ".mod";
    try {
      reader = new BufferedReader(new FileReader(file));
      writer = new PrintWriter(new BufferedWriter(new FileWriter(modifiedFileName)));
      while ((line = reader.readLine()) != null) {
        String msisdn = line.trim();
        if (subscriptionManager.subscribed(msisdn)) {
          writer.print(msisdn);
          writer.print("|");
          writer.print(question);
          writer.println();
        }
      }
      writer.flush();
      distribution.setFilePath(modifiedFileName);
    } catch (IOException e) {
      logger.error("Unable to modified abonents list: ", e);
      throw new QuizException("Unable to modified abonents list: ", e);
    } catch (SubManagerException e) {
      logger.error("Unable to modified abonents list: ", e);
      throw new QuizException("Unable to modified abonents list: ", e);
    } finally {
      if (writer != null) {
        writer.close();
      }
      if (reader != null) {
        try {
          reader.close();
        } catch (IOException e) {
          logger.error("Unabe to close file: ", e);
        }
      }
    }
  }

  private void writeError(String quizFileName, Throwable exc) {
    if (quizFileName == null) {
      return;
    }
    File file = new File(quizFileName);
    String quizName = file.getName().substring(0, file.getName().lastIndexOf("."));
    String errorFile = quizDir + File.separator + quizName + ".error";
    PrintWriter writer = null;
    try {
      writer = new PrintWriter(new BufferedWriter(new FileWriter(errorFile, true)));
      writer.println("Error during creating quiz:");
      exc.printStackTrace(writer);
      writer.flush();
      dirListener.remove(quizFileName,true);
      String statusFile = dirWork + File.separator + quizName + ".error";
      if(new File(statusFile).exists()) {
        dirListener.remove(statusFile,true);        
      }
    } catch (Exception e) {
      logger.error("Unable to create error file: " + errorFile, e);
    } finally {
      if (writer != null) {
        writer.close();
      }
    }
  }

  private void resolveConflict(Quiz quiz) throws QuizException {
    if (quiz == null) {
      logger.error("Some arguments are null");
      throw new QuizException("Some arguments are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
    }

    File file = new File(quiz.getFileName());
    String quizName = file.getName().substring(0, file.getName().lastIndexOf("."));
    String errorFile = quizDir + File.separator + quizName + ".error";
    file = new File(errorFile);
    if (!file.exists()) {
      return;
    }
    PrintWriter writer = null;
    try {
      writer = new PrintWriter(new BufferedWriter(new FileWriter(errorFile, true)));
      String sep = System.getProperty("line.separator");
      writer.write(sep);
      writer.write(sep);
      writer.println("Conflicts resolved...");
      writer.flush();
    } catch (IOException e) {
      logger.error("Unable to create error file: " + errorFile, e);
      throw new QuizException("Unable to create error file: " + errorFile, e);
    } finally {
      if (writer != null) {
        writer.close();
      }
    }
  }

  private void writeQuizesConflict(Quiz prevQuiz, Quiz newQuiz) throws QuizException {
    if ((prevQuiz == null) || (newQuiz == null)) {
      logger.error("Some arguments are null");
      throw new QuizException("Some arguments are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    logger.warn("Conflict quizes");
    File newQuizfile = new File(newQuiz.getFileName());
    String newQuizName = newQuizfile.getName().substring(0, newQuizfile.getName().lastIndexOf("."));
    PrintWriter writer = null;
    String errorFile = quizDir + File.separator + newQuizName + ".error";
    try {
      writer = new PrintWriter(new BufferedWriter(new FileWriter(errorFile, true)));
      writer.println(" Quizes conflict:");
      writer.println();
      writer.println("Previous quiz");
      writer.println(prevQuiz);
      writer.println();
      writer.println("New quiz");
      writer.println(newQuiz);
      writer.flush();
      dirListener.remove(newQuiz.getFileName(),true);
    } catch (IOException e) {
      logger.error("Unable to create error file: " + errorFile, e);
      throw new QuizException("Unable to create error file: " + errorFile, e);
    } finally {
      if (writer != null) {
        writer.close();
      }
    }
    dirListener.remove(newQuiz.getFileName(),true);
  }

  public MBeanServer getMBeansServer() throws QuizException {
    final MBeanServer mbs = ManagementFactory.getPlatformMBeanServer();
    try {
      mbs.registerMBean(dirListener.getMonitor(), new ObjectName("SMSQUIZ.quizmanager:mbean=dirListener"));
      mbs.registerMBean(replyStatsDataSource.getMonitor(), new ObjectName("SMSQUIZ.quizmanager:mbean=replystatsdsource"));
//      mbs.registerMBean(distributionManager.getMonitor(), new ObjectName("SMSQUIZ.quizmanager:mbean=distributionmanager"));
      mbs.registerMBean(subscriptionManager.getMonitor(), new ObjectName("SMSQUIZ.quizmanager:mbean=subscriptionManager"));
      mbs.registerMBean(quizBuilder.getMonitor(), new ObjectName("SMSQUIZ.quizmanager:mbean=quizBuilder"));
      mbs.registerMBean(monitor, new ObjectName("SMSQUIZ.quizmanager:mbean=quizManager"));
      final LoggingMBean lb = new LoggingMBean("SMSQUIZ", LogManager.getLoggerRepository());
      mbs.registerMBean(lb, new ObjectName("SMSQUIZ:mbean=logging"));

    } catch (Exception e) {
      e.printStackTrace();
      logger.error(e, e);
      throw new QuizException(e);
    }
    return mbs;
  }


  public int countQuizes() {
    return quizesMap.size();
  }

  public String getAvailableQuizes() {
    return quizesMap.values().toString();
  }

  public String getStatusDir() {
    return statusDir;
  }

  public String getDirResult() {
    return dirResult;
  }

  String getQuizDir() {
    return quizDir;
  }

  Long getListenerDelayFirst() {
    return listenerDelayFirst;
  }

  Long getListenerPeriod() {
    return listenerPeriod;
  }

  Long getCollectorDelayFirst() {
    return collectorDelayFirst;
  }

  Long getCollectorPeriod() {
    return collectorPeriod;
  }

  String getDirWork() {
    return dirWork;
  }
  
  private class QuizCreator extends Thread{
    private Quiz quiz;
    private Distribution distr;
    public QuizCreator(Quiz quiz, Distribution distr) {
      this.quiz = quiz;
      this.distr = distr;
    }

    public void run() {
      try{
        if(!new File(quiz.getFileName()).exists()) {
          writeError(quiz.getFileName(), new Exception("Error during creating quiz: quiz's file was deleted "+quiz.getFileName()));
          logger.error("Error during creating quiz: quiz's file was deleted "+quiz.getFileName());
          System.out.println("Error during creating quiz: quiz's file was deleted "+quiz.getFileName());
          return;
        }
        Quiz previousQuiz;
        if ((previousQuiz = quizesMap.get(quiz.getDestAddress())) != null) {
          logger.error("Error during creating quiz: quizes conflict");
          System.out.println("Error during creating quiz: quizes conflict");
          writeQuizesConflict(previousQuiz, quiz);
          return;
        }
        makeSubscribedOnly(distr, quiz.getQuestion(), quiz.getQuizName());
        String id;
        if ((id = quiz.getId()) != null) {
          if (logger.isInfoEnabled()) {
            logger.info("Quizes status will be repaired, current id: " + id);
          }

          id = distributionManager.repairStatus(id, quiz.getFileName() + ".distr.error",
              new QuizManagerTask(quizesMap, quiz), distr);

          if (logger.isInfoEnabled()) {
            logger.info("Quizes status repaired, new id: " + id);
          }
        } else {
          try {
            id = distributionManager.createDistribution(distr,
                new QuizManagerTask(quizesMap, quiz), quiz.getFileName() + ".distr.error");
          } catch (DistributionException e) {
            logger.error("Unable to create distribution", e);
            throw new QuizException("Unable to create distribution", e);
          }
        }
        quiz.setId(id);
        resolveConflict(quiz);
        if (logger.isInfoEnabled()) {
          logger.info("Quiz created: " + quiz);
        }
      } catch (Throwable e) {
        writeError(quiz.getFileName(), e);
        logger.error("Error during creating quiz",e);
        e.printStackTrace();
      }

    }
  }

}
