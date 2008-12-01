package mobi.eyeline.smsquiz.quizmanager;

import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.jmx.log4j.LoggingMBean;
import mobi.eyeline.smsquiz.distribution.Distribution;
import mobi.eyeline.smsquiz.distribution.DistributionException;
import mobi.eyeline.smsquiz.distribution.DistributionManager;
import mobi.eyeline.smsquiz.quizmanager.filehandler.DirListener;
import mobi.eyeline.smsquiz.quizmanager.filehandler.Notification;
import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;
import mobi.eyeline.smsquiz.quizmanager.quiz.QuizBuilder;
import mobi.eyeline.smsquiz.quizmanager.quiz.QuizError;
import mobi.eyeline.smsquiz.quizmanager.quiz.Status;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyStatsDataSource;
import mobi.eyeline.smsquiz.subscription.SubscriptionManager;
import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

import javax.management.MBeanServer;
import javax.management.ObjectName;
import java.io.*;
import java.lang.management.ManagementFactory;
import java.util.Collection;
import java.util.LinkedList;
import java.util.Observable;
import java.util.Observer;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ThreadFactory;

/**
 * author: alkhal
 */
@SuppressWarnings({"EmptyCatchBlock", "ResultOfMethodCallIgnored"})
public class QuizManager implements Observer {

  private static final Logger logger = Logger.getLogger(QuizManager.class);
  private static QuizManager quizManager;


  private DirListener dirListener;
  private QuizCollector quizCollector;
  private ReplyStatsDataSource replyStatsDataSource;
  private DistributionManager distributionManager;
  private SubscriptionManager subscriptionManager;        //init ConnectionPoll for DB needed

  private ScheduledExecutorService scheduledDirListener;
  private ScheduledExecutorService scheduledStatusChecker;
  private QuizBuilder quizBuilder;

  private QuizManagerMBean monitor;

  private ConcurrentHashMap<String, Quiz> quizesMap;
  private String quizDir;
  private long listenerDelayFirst;
  private long listenerPeriod;
  private long collectorDelayFirst;
  private long collectorPeriod;
  private long checkerFirstDelay;
  private long checkerPeriod;
  private String dirResult;
  private String dirWork;
  private String archiveDir;

  private Quizes quizes = new Quizes();


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
      listenerDelayFirst = config.getLong("listener_delay", 30);
      listenerPeriod = config.getLong("listener_period", 30);
      collectorDelayFirst = config.getLong("collector_delay", 30);
      collectorPeriod = config.getLong("collector_period", 30);
      checkerFirstDelay = config.getLong("status_checker_delay", 60);
      checkerPeriod = config.getLong("status_checker_period", 60);
      quizDir = config.getString("dir_quiz");
      archiveDir = config.getString("dir_archive");
      datePattern = "dd.MM.yyyy HH:mm";
      timePattern = "HH:mm";
      dirResult = config.getString("dir_result", "quizResults");
      dirWork = config.getString("dir_work", "quizManager_ab_mod");

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

    scheduledStatusChecker = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "DistributionStatusChecker");
      }
    });

    quizBuilder = new QuizBuilder(datePattern, timePattern);
    quizesMap = new ConcurrentHashMap<String, Quiz>();
    quizCollector = new QuizCollector(quizesMap, quizes);

    File file = new File(dirWork);
    if (!file.exists()) {
      file.mkdirs();
    }
    monitor = new QuizManagerMBean(this);
  }

  public void start() {
    scheduledDirListener.scheduleAtFixedRate(dirListener, listenerDelayFirst, listenerPeriod, java.util.concurrent.TimeUnit.SECONDS);
    scheduledStatusChecker.scheduleAtFixedRate(new DistributionStatusChecker(distributionManager, quizes, quizesMap),
        checkerFirstDelay, checkerPeriod, java.util.concurrent.TimeUnit.SECONDS);
    quizCollector.start();
  }

  public void stop() {
    scheduledDirListener.shutdown();
    replyStatsDataSource.shutdown();
    subscriptionManager.shutdown();
    distributionManager.shutdown();
    scheduledStatusChecker.shutdown();
  }

  public Result handleSms(String address, String oa, String text) throws QuizException {
    if (logger.isInfoEnabled()) {
      logger.info("Manager handle sms:" + address + " " + oa + " " + text);
    }
    Quiz quiz = quizesMap.get(address);
    if ((quiz != null) && (quiz.isGenerated()) && (quiz.isActive())) {
      return quiz.handleSms(oa, text);
    }
    logger.warn("Active and generated Quiz not found for address: " + address);
    if (logger.isInfoEnabled()) {
      logger.info("Available addresses: " + quizesMap.keySet().toString());
    }
    return null;
  }

  public void update(Observable o, Object arg) {
    try {
      logger.info("Updating quizfiles list...");
      Collection<Notification> ns = (Collection<Notification>) arg;
      for (Notification notification : ns) {
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
        } else if (notification.getStatus().equals(Notification.FileStatus.DELETED)) {
          try {
            deleteQuiz(notification);
          }
          catch (Exception e) {
            logger.error("Unable to delete quize: " + notification.getFileName());
          }
        }
      }
      quizCollector.alert();
    }
    catch (Throwable e) {
      logger.error(e, e);
    } finally {
      logger.info("Updating completed.");
    }
  }

  @SuppressWarnings({"ThrowableInstanceNeverThrown"})
  private void deleteQuiz(Notification notification) throws QuizException {
    Quiz quiz;
    try {
      String fileName = notification.getFileName();
      quiz = quizes.getQuizByFile(fileName);
      if (quiz != null) {
        quizes.remove(quiz);
        Quiz foundQuiz = quizesMap.get(quiz.getDestAddress());
        if (foundQuiz == quiz) {
          quizesMap.remove(quiz.getDestAddress());
        } else {
          if (logger.isInfoEnabled()) {
            logger.warn("Found quiz not equals removed quiz");
            logger.warn("Removed " + quiz);
            logger.warn("Found " + foundQuiz);
          }
        }

        quiz.shutdown();

        String taskId = quiz.getDistrId();

        if (taskId != null) {
          try {
            distributionManager.removeDistribution(taskId);
          } catch (Exception e) {
            logger.error(e, e);
          }
        }
        quiz.remove();

      }
    }
    catch (Throwable e) {
      writeError(notification.getFileName(), e);
      throw new QuizException(e);
    }
  }


  private void modifyQuiz(Notification notification) throws QuizException {

    String fileName = notification.getFileName();

    Quiz quiz;
    quiz = quizes.getQuizByFile(fileName);

    if (quiz == null) {
      createQuiz(notification);
      return;
    }

    try {
      if (quiz.isActive()) {
        quizBuilder.buildModifyActive(fileName, quiz);
      } else {
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

      quiz = new Quiz(file, replyStatsDataSource, distributionManager, dirResult, dirWork, archiveDir, quizDir);
      quizBuilder.buildQuiz(fileName, quiz);

      if (!quiz.isFinished()) {
        ConflictVisitor visitor = new ConflictVisitor(quiz);
        quizes.visit(visitor);
        for (Quiz prev : visitor.getConflicts()) {
          if (prev != null) {
            if ((prev.getDateBegin().compareTo(quiz.getDateEnd()) <= 0)
                && (quiz.getDateBegin().compareTo(prev.getDateEnd()) <= 0)) {
              logger.error("Error during creating quiz: quizes conflict");
              writeQuizesConflict(prev, quiz.getFileName());
              try {
                quiz.setError(QuizError.QUIZES_CONFLICT, "Quiz for one or more of this addresses already exists");
              } catch (QuizException e) {
              }
              return;
            }
          }
        }
        try {
          try {
            quiz.setQuizStatus(Status.QuizStatus.GENERATION);
            createDistribution(quiz);
          } catch (QuizException e) {
            logger.error(e, e);
            return;
          }
        } catch (Throwable e) {
          logger.error(e, e);
          throw new QuizException(e.toString(), e);
        }
      } else {
        if (quiz.getDistrId() == null) {
          logger.warn("Quiz is finished, but it's distribution id doesn't exist: " + quiz);
          quiz.setExported(true);
        }
      }
      quizes.add(quiz);

    } catch (QuizException e) {
      if (quiz != null) {
        try {
          quiz.setError(QuizError.CREATE_ERROR, e.getMessage());
        } catch (Exception ex) {
        }
      }
      logger.error(e, e);
      writeError(notification.getFileName(), e);
      throw new QuizException(e);
    }
  }

  private void writeError(String quizFileName, Throwable exc) {
    if (quizFileName == null) {
      return;
    }
    File file = new File(quizFileName);
    String quizName = file.getName().substring(0, file.getName().lastIndexOf("."));
    String errorFile = dirWork + File.separator + quizName + ".error";
    PrintWriter writer = null;
    try {
      writer = new PrintWriter(new BufferedWriter(new FileWriter(errorFile, true)));
      writer.println("Error during creating quiz:");
      exc.printStackTrace(writer);
      writer.flush();
    } catch (Exception e) {
      logger.error("Unable to create error file: " + errorFile, e);
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
    String errorFile = dirWork + File.separator + newQuizName + ".error";
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

  @SuppressWarnings({"ThrowableInstanceNeverThrown"})
  private void createDistribution(final Quiz quiz) throws QuizException {
    try {
      if (logger.isInfoEnabled()) {
        logger.info("Create distribution...");
      }

      String id;
      DistributionManager.State state;
      boolean create = true;
      if ((id = quiz.getDistrId()) != null) {
        if (logger.isInfoEnabled()) {
          logger.info("Quiz will be repaired: " + quiz);
        }
        state = distributionManager.getState(id);
        if (!state.equals(DistributionManager.State.ERROR)) {
          create = false;
        }
      }

      try {
        if (create) {
          Distribution distr = buildDistribution(quiz);
          id = distributionManager.createDistribution(distr);
        }
      } catch (DistributionException e) {
        logger.error("Unable to create distribution", e);
        throw new QuizException("Unable to create distribution", e);
      }

      quiz.setDistrId(id);
      if (logger.isInfoEnabled()) {
        logger.info("Quiz created: " + quiz);
      }

    } catch (Throwable e) {
      try {
        quiz.setError(QuizError.DISTR_ERROR, e.getMessage());
      } catch (QuizException ex) {
      }
      writeError(quiz.getFileName(), e);
      logger.error(e, e);
      throw new QuizException(e.toString(), e);
    } finally {
      if (logger.isInfoEnabled()) {
        logger.info("Creating quiz completed.");
      }
    }
  }

  private Distribution buildDistribution(Quiz quiz) throws QuizException {
    Distribution distr = new DistributionImpl(quiz.getOrigAbFile(), subscriptionManager);
    distr.setQuestion(quiz.getQuestion());
    distr.setDateBegin(quiz.getDateBegin());
    distr.setDateEnd(quiz.getDistrDateEnd());
    distr.setSourceAddress(quiz.getSourceAddress());
    distr.setTaskName(quiz.getQuizName() + "(SmsQuiz)");
    distr.setTimeBegin(quiz.getTimeBegin());
    distr.setTimeEnd(quiz.getTimeEnd());
    distr.setTxmode(quiz.isTxmode());
    distr.addDays(quiz.getDays());
    return distr;
  }

  public long getCheckerFirstDelay() {
    return checkerFirstDelay;
  }

  public long getCheckerPeriod() {
    return checkerPeriod;
  }

  private class ConflictVisitor implements Quizes.Visitor {

    private Collection<Quiz> conflicts = new LinkedList<Quiz>();

    private final Quiz newcomer;

    private ConflictVisitor(Quiz newcomer) {
      if (newcomer == null) {
        logger.error("Some arguments are null");
        throw new IllegalArgumentException("Some arguments are null");
      }
      this.newcomer = newcomer;
    }

    public void visit(Quiz quiz) throws QuizException {
      if (quiz.getDestAddress().equals(newcomer.getDestAddress())) {
        conflicts.add(quiz);
      }
    }

    public Collection<Quiz> getConflicts() {
      return conflicts;
    }
  }

}
