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
import mobi.eyeline.smsquiz.quizmanager.quiz.QuizError;
import mobi.eyeline.smsquiz.quizmanager.quiz.Status;
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
import java.util.concurrent.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;

/**
 * author: alkhal
 */
@SuppressWarnings({"EmptyCatchBlock", "EmptyCatchBlock"})
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

  private Lock lock;
  private Condition notUpdate;
  private Condition notRun;

  private int run = -1;
  private int update = -1;

  private ThreadPoolExecutor removeExecutor;

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
    lock = new ReentrantLock();
    notUpdate = lock.newCondition();
    notRun = lock.newCondition();

    removeExecutor = new ThreadPoolExecutor(1,10,30,TimeUnit.SECONDS, new LinkedBlockingQueue<Runnable>(), new ThreadFactory(){
      public Thread newThread(Runnable r) {
        return new Thread(r,"FileRemover");
      }
    });
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
    if ((quiz != null)&&(quiz.isGenerates())&&(quiz.isActive())) {
      return quiz.handleSms(oa, text);
    }
    logger.warn("Active and generated Quiz not found for address: " + address);
    if (logger.isInfoEnabled()) {
      logger.info("Available addresses: " + quizesMap.keySet().toString());
    }
    return null;
  }

  public void update(Observable o, Object arg) {
    try{
      lock.lock();
      if (run == 1) {
        notRun.await();
      }
      update = 1;
      logger.info("Updating quizfiles list... Lock");
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
    }
    catch (Throwable e) {
      logger.error(e,e);
    } finally {
      update = 0;
      notUpdate.signal();
      lock.unlock();
      logger.info("Updating completed. Unlock.");
    }
  }

  @SuppressWarnings({"ThrowableInstanceNeverThrown"})
  private void deleteQuiz(Notification notification) throws QuizException{
    Quiz quiz = null;
    try {
      String fileName = notification.getFileName();
      for (Map.Entry<String,Quiz> e : quizesMap.entrySet()) {
        if (e.getValue().getFileName().equals(fileName)) {
          quiz = e.getValue();
          if(!quiz.isActive()) {
            quizesMap.remove(e.getKey());
          }
          break;
        }
      }
      if(quiz!=null) {
        quizesMap.remove(quiz.getDestAddress());
        String taskId = quiz.getId();
        if(taskId!=null) {
          distributionManager.removeTask(taskId);
        }
      }
    }
    catch (Throwable e) {
      writeError(notification.getFileName(), e);
      throw new QuizException(e);
    }
  }

  private void modifyQuiz(Notification notification) throws QuizException {

    String fileName = notification.getFileName();

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

    try {
      if(quiz.isActive()) {
        quizBuilder.buildModifyActive(fileName, quiz);
      }else {
        quizBuilder.buildModifyUnactive(fileName, quiz);
      }
    } catch (QuizException e) {
      writeError(notification.getFileName(), e);
      quiz.setError(QuizError.CREATE_ERROR, "Parsing exception during modification");
      throw e;
    }
    if (logger.isInfoEnabled()) {
      logger.info("Quiz modified: " + fileName);
    }
  }

  private void createQuiz(final Notification notification) throws QuizException {
    if (logger.isInfoEnabled()) {
      logger.info("Create quiz...");
    }
    Quiz quiz = null;
    try {
      String fileName = notification.getFileName();
      File file = new File(fileName);

      quiz = new Quiz(statusDir, file, new Distribution(),
          replyStatsDataSource, distributionManager, dirResult, dirWork);
      quizBuilder.buildQuiz(fileName, quiz);

      Quiz previousQuiz = quizesMap.get(quiz.getDestAddress());
      if(previousQuiz!=null) {
        logger.error("Error during creating quiz: quizes conflict");
        writeQuizesConflict(previousQuiz, quiz.getFileName());
        try{
          quiz.setError(QuizError.QUIZES_CONFLICT,"Quiz for this destaination address already exists");
        }catch (QuizException e) {}
        return;
      }
      quizesMap.put(quiz.getDestAddress(), quiz);
      try{
        QuizCreator quizCreator = new QuizCreator(quiz.getDestAddress(), quiz.getFileName());
        try{
          quiz.setQuizStatus(Status.QuizStatus.GENERATION);
        }catch (QuizException e) {
          logger.error(e,e);
        }
        if(quiz.isActive()) {
          quizCreator.start();
        } else {
          long delay = quiz.getDateBegin().getTime() - System.currentTimeMillis();
          ScheduledExecutorService executor = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
            public Thread newThread(Runnable r) {
              return new Thread(r,"QuizCreator: "+notification.getFileName());
            }
          });
          executor.schedule(quizCreator,delay,java.util.concurrent.TimeUnit.MILLISECONDS);
        }
      }catch (Throwable e) {
        quizesMap.remove(quiz.getDestAddress());
        logger.error(e,e);
        throw new QuizException(e.toString(),e);
      }
    }
    catch (QuizException e) {
      if(quiz!=null) {
        try{
          quiz.setError(QuizError.CREATE_ERROR,e.getMessage());
        }catch (Exception ex) {}
      }
      logger.error(e,e);
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
    String modifiedFileName = dirWork + File.separator + quizId + "(SmsQuiz)" + ".mod";
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
      try{
        removeExecutor.execute(new FileRemover(quizFileName,true));
      }catch (Exception e) {
        logger.error(e,e);
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

  private void writeQuizesConflict(Quiz prevQuiz, String newQuizFileName) throws QuizException {
    if ((prevQuiz == null) || (newQuizFileName == null)) {
      logger.error("Some arguments are null");
      throw new QuizException("Some arguments are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    logger.warn("Conflict quizes");
    File newQuizfile = new File(newQuizFileName);
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
      writer.println(newQuizFileName);
      writer.flush();
      try{
        removeExecutor.execute(new FileRemover(newQuizFileName,true));
      }catch (Exception e) {
        logger.error(e,e);
      }
    } catch (IOException e) {
      logger.error("Unable to create error file: " + errorFile, e);
      throw new QuizException("Unable to create error file: " + errorFile, e);
    } finally {
      if (writer != null) {
        writer.close();
      }
    }
  }

  public MBeanServer getMBeansServer() throws QuizException {
    final MBeanServer mbs = ManagementFactory.getPlatformMBeanServer();
    try {
      mbs.registerMBean(dirListener.getMonitor(), new ObjectName("SMSQUIZ.quizmanager:mbean=dirListener"));
      mbs.registerMBean(replyStatsDataSource.getMonitor(), new ObjectName("SMSQUIZ.quizmanager:mbean=replystatsdsource"));
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

    private String destAddress;

    private String fileName;

    public QuizCreator(String destAddress, String fileName) throws IllegalArgumentException{
      if((destAddress==null)||(fileName==null)) {
        logger.error("Some arguments are null");
        throw new IllegalArgumentException("Some arguments are null") ;
      }
      this.destAddress = destAddress;
      this.fileName = fileName;
    }

    @SuppressWarnings({"ThrowableInstanceNeverThrown"})
    public void run() {
      try{
        lock.lock();
        if (update == 1) {
          notUpdate.await();
        }
        run = 1;
        if(logger.isInfoEnabled()) {
          logger.info("Runing quizCreator. Lock.");
        }

        Quiz quiz = quizesMap.get(destAddress);
        if((!new File(fileName).exists())||(quiz == null)) {
          logger.warn("Error during creating quiz: quiz's file was deleted "+fileName);
          return;
        }

        try{
          Distribution distr = quiz.getDistribution();
          makeSubscribedOnly(distr, quiz.getQuestion(), quiz.getQuizName());

          String id;
          if ((id = quiz.getId()) != null) {
            if (logger.isInfoEnabled()) {
              logger.info("Quizes status will be repaired, current id: " + id);
            }

            id = distributionManager.repairStatus(id, quiz.getFileName() + ".distr.error",
                new QuizManagerTask(quiz), distr);

            if (logger.isInfoEnabled()) {
              logger.info("Quizes status repaired, new id: " + id);
            }
          } else {
            try {
              id = distributionManager.createDistribution(distr,
                  new QuizManagerTask(quiz), quiz.getFileName() + ".distr.error");
            } catch (DistributionException e) {
              logger.error("Unable to create distribution", e);
              throw new QuizException("Unable to create distribution", e);
            }
          }
          quiz.setId(id);
          try{
            quiz.setQuizStatus(Status.QuizStatus.AWAIT);
          }catch (QuizException e){
            logger.error(e,e);
          }
          resolveConflict(quiz);
          if (logger.isInfoEnabled()) {
            logger.info("Quiz created: " + quiz);
          }
        } catch (Throwable e) {
          try{
            quiz.setError(QuizError.DISTR_ERROR, e.getMessage());
          }catch (QuizException ex) {}
          quizesMap.remove(destAddress);
          throw e;
        }
      } catch (Throwable e) {
        writeError(fileName, e);
        logger.error(e,e);
      } finally {
        run = 0;
        notRun.signal();
        lock.unlock();
        if(logger.isInfoEnabled()) {
          logger.info("Creating quiz completed. Unlock.");
        }
      }

    }
  }
  private class FileRemover extends Thread{
    private String filename;
    private boolean rename;

    public FileRemover(String filename, boolean rename) {
      this.filename = filename;
      this.rename = rename;
    }
    public void run() {
      try {
        dirListener.remove(filename, rename);
      } catch (Exception e) {
        logger.error(e,e);
      }
    }
  }

}
