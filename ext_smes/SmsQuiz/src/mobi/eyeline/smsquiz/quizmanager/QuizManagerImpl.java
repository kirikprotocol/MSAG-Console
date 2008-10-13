package mobi.eyeline.smsquiz.quizmanager;

import mobi.eyeline.smsquiz.distribution.DistributionManager;
import mobi.eyeline.smsquiz.distribution.DistributionException;
import mobi.eyeline.smsquiz.distribution.Distribution;
import mobi.eyeline.smsquiz.quizmanager.dirlistener.DirListener;
import mobi.eyeline.smsquiz.quizmanager.dirlistener.Notification;
import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;
import mobi.eyeline.smsquiz.quizmanager.quiz.QuizBuilder;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyStatsDataSource;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyDataSourceException;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;

import java.io.*;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.Executors;
import java.util.*;

import org.apache.log4j.Logger;

/**
 * author: alkhal
 */
public class QuizManagerImpl implements QuizManager, Observer {

    private static Logger logger = Logger.getLogger(QuizManagerImpl.class);
    private static QuizManager quizManager;
    private static DistributionManager distributionManager;


    private Map<String,Quiz> quizesMap;
    private DirListener dirListener;
    private ReplyStatsDataSource replyStatsDataSource;
    private ScheduledExecutorService scheduledExecutor;
    private QuizBuilder quizBuilder;

    private String quizDir;
    private String statusDir;
    private long listenerDelayFirst;
    private long listenerPeriod;

    public static void init (final String configFile) throws QuizException{
        quizManager = new QuizManagerImpl(configFile);
    }

    public static QuizManager getInstanse() {
        return quizManager;
    }

    private QuizManagerImpl(final String configFile) throws QuizException{
        String datePattern;
        String timePattern;
        try {
            final XmlConfig c = new XmlConfig();
            c.load(new File(configFile));
            PropertiesConfig config = new PropertiesConfig(c.getSection("quizmanager").toProperties("."));
            listenerDelayFirst = config.getLong("listener.delay.first");
            listenerPeriod = config.getLong("listener.period.repeat");
            quizDir = config.getString("dir.quiz");
            statusDir = config.getString("dir.status");
            datePattern = config.getString("quiz.date.pattern");
            timePattern = config.getString("quiz.time.pattern");

        } catch (ConfigException e) {
            logger.error("Unable to construct QuizManagerImpl",e);
            throw new QuizException("Unable to construct QuizManagerImpl",e);
        }
        try {
            dirListener = new DirListener(quizDir);
        } catch (Exception e) {
            logger.error("Unable to construct DirListener",e);
            throw new QuizException("Unable to construct DirListener",e);
        }
        try {
            distributionManager = QuizManagerFieldFactory.getDistributionManager(configFile);
        } catch (DistributionException e) {
            logger.error("Unable to construct distributionManager",e);
            throw new QuizException("Unable to construct distributionManager",e);
        }
        try {
            replyStatsDataSource = QuizManagerFieldFactory.getReplyStatsDataSource(configFile);
        } catch (ReplyDataSourceException e) {
            logger.error("Unable to construct replyStatsDataSource",e);
            throw new QuizException("Unable to construct replyStatsDataSource",e);
        }

        dirListener.addObserver(this);
        scheduledExecutor = Executors.newSingleThreadScheduledExecutor();
        quizBuilder = new QuizBuilder(datePattern, timePattern);
        quizesMap = new HashMap<String, Quiz>();

        File file = new File(statusDir);
        if(!file.exists()) {
            file.mkdirs();
        }
 }

    public void start() {
        scheduledExecutor.scheduleAtFixedRate(dirListener, listenerDelayFirst, listenerPeriod, java.util.concurrent.TimeUnit.SECONDS);
    }

    public void stop() {
        scheduledExecutor.shutdown();
    }

    public Result handleSms(String address, String oa, String text) {
        return null;       //todo
    }

    public void update(Observable o, Object arg) {
        logger.info("Updating quizfiles list...");
        Notification notification = (Notification)arg;
        if(notification.getStatus().equals(Notification.FileStatus.MODIFIED)) {
            try{
                modifyQuiz(notification);
            } catch (QuizException e) {
                logger.error("Unable to modify quiz: "+notification.getFileName());
            }
            return;
        }
        if(notification.getStatus().equals(Notification.FileStatus.CREATED)) {
            try {
                createQuiz(notification);
            } catch (QuizException e) {
                logger.error("Unable to modify quiz: "+notification.getFileName());
            }
        }
        logger.info("Updating finished");
    }

    private void modifyQuiz (Notification notification) throws QuizException{
        Distribution distribution = new Distribution();
        String fileName = notification.getFileName();
        File file = new File(fileName);

        String quizName = file.getName().substring(0,file.getName().lastIndexOf("."));
        String statsFileName = statusDir+"/"+quizName+".status";
        File statsFile = new File(statsFileName);

        Quiz quiz = quizesMap.get(notification.getFileName());
        if(quiz==null) {
           logger.error("Can't find quiz into map with key: "+notification.getFileName());
           throw new QuizException("Can't find quiz into map with key: "+notification.getFileName());
        }
        quizBuilder.buildQuiz(fileName, distribution, quiz);
        try {
            String id = distributionManager.createDistribution(distribution);         //todo?
            quiz.setId(id);
        } catch (DistributionException e) {
           logger.error("Unable to create distribution", e);
           throw new QuizException("Unable to create distribution", e);
        }

    }
    private void createQuiz (Notification notification) throws QuizException {
        String fileName = notification.getFileName();
        Distribution distribution = null;
        File file = new File(fileName);
        Quiz quiz = new Quiz(statusDir, file, replyStatsDataSource);
        if(quiz.getId()!=null) {
            quizBuilder.buildQuiz(notification.getFileName(),null,quiz);
        }
        else {
            distribution = new Distribution();
            quizBuilder.buildQuiz(fileName,distribution,quiz);
            String id = null;
            try {
                id = distributionManager.createDistribution(distribution);
            } catch (DistributionException e) {
                logger.error("Unable to create distribution",e);
                throw new QuizException("Unable to create distribution",e);
            }
            quiz.setId(id);
        }
        quizesMap.put(fileName, quiz);

    }

    public int countQuizes() {
        return quizesMap.size();
    }

    public String getStatusDir() {
        return statusDir;
    }

}
