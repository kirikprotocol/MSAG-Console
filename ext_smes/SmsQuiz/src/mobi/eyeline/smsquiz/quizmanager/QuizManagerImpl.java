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
    private QuizCollector quizCollector;
    private ReplyStatsDataSource replyStatsDataSource;
    private ScheduledExecutorService scheduledDirListener;
    private ScheduledExecutorService scheduledQuizCollector;
    private QuizBuilder quizBuilder;

    private String quizDir;
    private String statusDir;
    private long listenerDelayFirst;
    private long listenerPeriod;
    private String dirResult;

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
            dirResult = config.getString("dir.result");

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
        quizCollector = new QuizCollector();
        dirListener.addObserver(this);
        scheduledDirListener = Executors.newSingleThreadScheduledExecutor();
        scheduledQuizCollector = Executors.newSingleThreadScheduledExecutor();
        quizBuilder = new QuizBuilder(datePattern, timePattern);
        quizesMap = new HashMap<String, Quiz>();

        File file = new File(statusDir);
        if(!file.exists()) {
            file.mkdirs();
        }
 }

    public void start() {
        scheduledDirListener.scheduleAtFixedRate(dirListener, listenerDelayFirst, listenerPeriod, java.util.concurrent.TimeUnit.SECONDS);
        scheduledQuizCollector.scheduleAtFixedRate(quizCollector,10000,10000,java.util.concurrent.TimeUnit.SECONDS);   //todo
    }

    public void stop() {
        scheduledDirListener.shutdown();
        scheduledQuizCollector.shutdown();
    }

    public Result handleSms(String address, String oa, String text) throws QuizException{
        Quiz quiz = null;
        if((quiz = quizesMap.get(address))!=null) {
            return quiz.handleSms(oa, text);
        }
        return null;
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
        try{
            Distribution distribution = new Distribution();
            String fileName = notification.getFileName();
            File file = new File(fileName);

            String quizName = file.getName().substring(0,file.getName().lastIndexOf("."));
            String statsFileName = statusDir+"/"+quizName+".status";
            File statsFile = new File(statsFileName);

            Quiz quiz = null;
            for(Quiz q:quizesMap.values()) {
                if(q.getFileName().equals(fileName)) {
                    quiz = q;
                }
            }
            if(quiz==null) {
               logger.error("Can't find quiz into map with key: "+notification.getFileName());
               throw new QuizException("Can't find quiz into map with key: "+notification.getFileName());
            }

            quizBuilder.buildQuiz(fileName, distribution, quiz);                  //todo?
        } catch (QuizException e) {
            writeError(notification.getFileName(),e);
            throw e;
        }
    }
    private void createQuiz (Notification notification) throws QuizException{
        String fileName = notification.getFileName();
        Distribution distribution = null;
        File file = new File(fileName);
        Quiz quiz = null;
        try {
            quiz = new Quiz(statusDir, file, replyStatsDataSource, distributionManager, dirResult);
            if(quiz.getId()!=null) {
                quizBuilder.buildQuiz(notification.getFileName(),null,quiz);
            }
            else {
                distribution = new Distribution();
                quizBuilder.buildQuiz(fileName,distribution,quiz);
            }
            Quiz previousQuiz = null;
            if((previousQuiz = quizesMap.get(quiz.getDestAddress()))!=null) {
                writeQuizesConflict(previousQuiz, quiz);
                return;
            }
            if(distribution!=null) {
                String id = null;
                try {
                    id = distributionManager.createDistribution(distribution);
                } catch (DistributionException e) {
                    logger.error("Unable to create distribution",e);
                    throw new QuizException("Unable to create distribution",e);
                }
                quiz.setId(id);
            }
            resolveConflict(quiz);
            quizesMap.put(quiz.getDestAddress(), quiz);
        }
        catch (QuizException e) {
            writeError(notification.getFileName(),e);
            throw e;
        }
    }

    private void writeError(String quizFileName, Exception exc) {
        if(quizFileName==null) {
            return;
        }
        File file = new File(quizFileName);
        String quizName = file.getName().substring(0,file.getName().lastIndexOf("."));
        String errorFile = quizDir+"/"+quizName+".error";
        PrintWriter writer = null;
        try {
            writer = new PrintWriter(new BufferedWriter(new FileWriter(errorFile,true)));
            writer.println("Error during creating quiz:");
            exc.printStackTrace(writer);
            writer.flush();
        } catch (IOException e) {
            logger.error("Unable to create error file: "+errorFile, e);
        } finally {
            if(writer!=null) {
                writer.close();
            }
        }
    }

    private void resolveConflict(Quiz quiz) throws QuizException{
        if(quiz==null) {
            logger.error("Some arguments are null");
            throw new QuizException("Some arguments are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
        }

        File file = new File(quiz.getFileName());
        String quizName = file.getName().substring(0,file.getName().lastIndexOf("."));
        String errorFile = quizDir+"/"+quizName+".error";
        file = new File(errorFile);
        if(!file.exists()) {
            return;
        }
        PrintWriter writer = null;
        try {
            writer = new PrintWriter(new BufferedWriter(new FileWriter(errorFile,true)));
            writer.println("\n\n\nConflicts resolved...");
            writer.flush();
        } catch (IOException e) {
            logger.error("Unable to create error file: "+errorFile, e);
            throw new QuizException("Unable to create error file: "+errorFile, e);
        } finally {
            if(writer!=null) {
                writer.close();
            }
        }
    }

    private void writeQuizesConflict(Quiz prevQuiz, Quiz newQuiz) throws QuizException{
        if((prevQuiz==null)||(newQuiz==null)) {
            logger.error("Some arguments are null");
            throw new QuizException("Some arguments are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        logger.warn("Conflict quizes");
        File newQuizfile = new File(newQuiz.getFileName());
        String newQuizName = newQuizfile.getName().substring(0,newQuizfile.getName().lastIndexOf("."));
        PrintWriter writer = null;
        String errorFile = quizDir+"/"+newQuizName+".error";
        try {
            writer = new PrintWriter(new BufferedWriter(new FileWriter(errorFile,true)));
            writer.println(" Quizes conflict:");
            writer.println();
            writer.println("Previous quiz");
            writer.println(prevQuiz);
            writer.println();
            writer.println("New quiz");
            writer.println(newQuiz);
            writer.flush();
        } catch (IOException e) {
            logger.error("Unable to create error file: "+errorFile, e);
            throw new QuizException("Unable to create error file: "+errorFile, e);
        } finally {
            if(writer!=null) {
                writer.close();
            }
        }
        dirListener.remove(newQuiz.getFileName(),false);
    }


    public int countQuizes() {
        return quizesMap.size();
    }

    public String getStatusDir() {
        return statusDir;
    }

}
