package mobi.eyeline.smsquiz.quizmanager;

import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.jmx.log4j.LoggingMBean;
import mobi.eyeline.smsquiz.distribution.DistributionManager;
import mobi.eyeline.smsquiz.quizmanager.filehandler.DirListener;
import mobi.eyeline.smsquiz.quizmanager.filehandler.Notification;
import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;
import mobi.eyeline.smsquiz.quizmanager.quiz.QuizBuilder;
import mobi.eyeline.smsquiz.quizmanager.quiz.QuizData;
import mobi.eyeline.smsquiz.quizmanager.quiz.QuizError;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyStatsDataSource;
import mobi.eyeline.smsquiz.subscription.SubscriptionManager;
import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

import javax.management.MBeanServer;
import javax.management.ObjectName;
import java.io.File;
import java.lang.management.ManagementFactory;
import java.util.*;
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
  private QuizBuilder quizBuilder;

  private QuizManagerMBean monitor;

  private ConcurrentHashMap<String, Quiz> quizesMap;
  private String quizDir;
  private long listenerDelayFirst;
  private long listenerPeriod;
  private long collectorDelayFirst;
  private long collectorPeriod;
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

    quizBuilder = new QuizBuilder(datePattern, timePattern);
    quizesMap = new ConcurrentHashMap<String, Quiz>();
    quizCollector = new QuizCollector(quizesMap, quizes, distributionManager);

    File file = new File(dirWork);
    if (!file.exists()) {
      file.mkdirs();
    }
    monitor = new QuizManagerMBean(this);
  }

  public void start() {
    scheduledDirListener.scheduleAtFixedRate(dirListener, listenerDelayFirst, listenerPeriod, java.util.concurrent.TimeUnit.SECONDS);
    quizCollector.start();
  }

  public void stop() {
    scheduledDirListener.shutdown();
    replyStatsDataSource.shutdown();
    subscriptionManager.shutdown();
    distributionManager.shutdown();
    quizCollector.shutdown();
  }

  public Result handleSms(String address, String oa, String text) throws QuizException {
    if (logger.isInfoEnabled()) {
      logger.info("Manager handle sms:" + address + " " + oa + " " + text);
    }
    Quiz quiz = quizesMap.get(address);
    if (quiz != null) {
      return quiz.handleSms(oa, text);
    }
    logger.warn("Active Quiz not found for address: " + address);
    if (logger.isInfoEnabled()) {
      logger.info("Available addresses: " + quizesMap.keySet().toString());
    }
    return new Result("", Result.ReplyRull.SERVICE_NOT_FOUND, "");
  }

  @SuppressWarnings({"unchecked"})
  public void update(Observable o, Object arg) {
    try {
      logger.info("Updating quizfiles list...");
      Collection<Notification> ns = (Collection<Notification>) arg;
      for (Notification notification : ns) {
        if (notification.getStatus().equals(Notification.FileStatus.MODIFIED)) {
          try {
            modifyQuiz(notification);
          } catch (Exception e) {
            logger.error("Unable to modify quiz: " + notification.getFileName(), e);
          }

        } else if (notification.getStatus().equals(Notification.FileStatus.CREATED)) {
          try {
            createQuiz(notification);
          } catch (Exception e) {
            logger.error("Unable to update quize: " + notification.getFileName(), e);
          }
        } else if (notification.getStatus().equals(Notification.FileStatus.DELETED)) {
          try {
            deleteQuiz(notification);
          }
          catch (Exception e) {
            logger.error("Unable to delete quize: " + notification.getFileName(), e);
          }
        }
      }
    }
    catch (Throwable e) {
      logger.error(e, e);
    } finally {
      logger.info("Updating completed.");
    }
  }

  @SuppressWarnings({"ThrowableInstanceNeverThrown"})
  private void deleteQuiz(Notification notification) throws QuizException {
    String fileName = notification.getFileName();
    Quiz quiz = quizes.getQuizByFile(fileName);
    try {
      if (quiz != null) {
        if (logger.isInfoEnabled()) {
          logger.info("Removing quiz from quizes: " + quiz);
        }
        quizes.remove(quiz);
        if (quiz.getDestAddress() != null) {
          Quiz foundQuiz = quizesMap.get(quiz.getDestAddress());
          if (foundQuiz == quiz) {
            quizesMap.remove(quiz.getDestAddress());
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
      if (quiz != null) {
        quiz.writeError(e);
      }
      throw new QuizException(e);
    }
  }


  private void modifyQuiz(Notification notification) throws QuizException {

    String fileName = notification.getFileName();

    Quiz quiz;
    quiz = quizes.getQuizByFile(fileName);

    try {
      if (quiz == null) {
        createQuiz(notification);
        return;
      }
      QuizData data = quizBuilder.buildQuiz(quiz.getFileName());
      quiz.updateQuiz(data);
    } catch (Exception e) {
      if (quiz != null) {
        quiz.writeError(e);
        quiz.setQuizStatus(QuizError.CREATE_ERROR, "Parsing exception during modification");
      }
      throw new QuizException(e);
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

      QuizData data = quizBuilder.buildQuiz(fileName);

      quiz = new Quiz(file, replyStatsDataSource, quizCollector, distributionManager, subscriptionManager,
          dirResult, dirWork, archiveDir, quizDir, data);

      validateDates(data);

      if (!new Date().after(data.getDateEnd())) {
        ConflictVisitor visitor = new ConflictVisitor(data.getDestAddress());
        quizes.visit(visitor);
        for (Quiz prev : visitor.getConflicts()) {
          if (prev != null) {
            if ((prev.getDateBegin().compareTo(data.getDateEnd()) <= 0)
                && (data.getDateBegin().compareTo(prev.getDateEnd()) <= 0)) {
              logger.error("Error during creating quiz: quizes conflict");
              quiz.writeQuizesConflict(fileName, prev);
              try {
                quiz.setQuizStatus(QuizError.QUIZES_CONFLICT, "Quiz for one or more of this addresses already exists");
              } catch (QuizException e) {
              }
              return;
            }
          }
        }
      }

      quizes.add(quiz);
      quiz.updateQuiz(data);

    } catch (Exception e) {
      if (quiz != null) {
        try {
          quiz.setQuizStatus(QuizError.CREATE_ERROR, "Unable to create quiz");
        } catch (Exception ex) {
        }
        quiz.writeError(e);
      }
      logger.error(e, e);
      throw new QuizException(e);
    }
  }

  private void validateDates(QuizData quiz) throws QuizException {
    Date dateBegin = quiz.getDateBegin();
    Date dateEnd = quiz.getDateEnd();
    Date distrEndDate = quiz.getDistrDateEnd();
    if (dateEnd.before(dateBegin)) {
      throw new QuizException("Invalid end date: " + dateEnd);
    }
    if (distrEndDate.before(dateBegin) || distrEndDate.after(dateEnd)) {
      throw new QuizException("Invalid distribution end date: " + distrEndDate);
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

  private class ConflictVisitor implements Quizes.Visitor {

    private Collection<Quiz> conflicts = new LinkedList<Quiz>();

    private final String newcomerDa;

    private ConflictVisitor(String newcomerDa) {
      if (newcomerDa == null) {
        throw new IllegalArgumentException("Some arguments are null");
      }
      this.newcomerDa = newcomerDa;
    }

    public void visit(Quiz quiz) throws QuizException {
      if (quiz.getDestAddress().equals(newcomerDa)) {
        conflicts.add(quiz);
      }
    }

    public Collection<Quiz> getConflicts() {
      return conflicts;
    }
  }

}
