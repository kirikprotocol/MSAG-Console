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
import mobi.eyeline.smsquiz.subscription.datasource.SubscriptionDataSource;
import mobi.eyeline.smsquiz.subscription.SubscriptionManager;
import mobi.eyeline.smsquiz.subscription.SubManagerException;
import mobi.eyeline.smsquiz.storage.StorageException;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;

import java.io.*;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ConcurrentHashMap;
import java.util.*;

import org.apache.log4j.Logger;

/**
 * author: alkhal
 */
public class QuizManagerImpl implements QuizManager, Observer { // todo remove interface

    private static Logger logger = Logger.getLogger(QuizManagerImpl.class);
    private static QuizManager quizManager;



    private DirListener dirListener;
    private QuizCollector quizCollector;
    private ReplyStatsDataSource replyStatsDataSource;
    private DistributionManager distributionManager;        //init ConnectionPoll for DB needed
    private SubscriptionManager subscriptionManager;

    private ScheduledExecutorService scheduledDirListener;
    private ScheduledExecutorService scheduledQuizCollector;
    private QuizBuilder quizBuilder;

    private ConcurrentHashMap<String,Quiz> quizesMap;
    private String quizDir;
    private String statusDir;
    private long listenerDelayFirst;
    private long listenerPeriod;
    private long collectorDelayFirst;
    private long collectorPeriod;
    private String dirResult;
    private String dirModifiedAb;

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
            collectorDelayFirst = config.getLong("collector.delay.first");
            collectorPeriod = config.getLong("collector.period.repeat");
            quizDir = config.getString("dir.quiz");
            statusDir = config.getString("dir.status");
            datePattern = config.getString("quiz.date.pattern");
            timePattern = config.getString("quiz.time.pattern");
            dirResult = config.getString("dir.result");
            dirModifiedAb = config.getString("dir.modified.abonents");

            File file = new File(dirModifiedAb);
            if(!file.exists()){
                file.mkdirs();
            }

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

        try {
            subscriptionManager = SubscriptionManager.getInstance();
        } catch (SubManagerException e) {
            logger.error("Unable to construct subscriptionDataSource",e);
            throw new QuizException("Unable to subscriptionDataSource",e);
        }

        dirListener.addObserver(this);
        scheduledDirListener = Executors.newSingleThreadScheduledExecutor();       // todo thread factory
        scheduledQuizCollector = Executors.newSingleThreadScheduledExecutor();     // todo thread factory
        quizBuilder = new QuizBuilder(datePattern, timePattern);
        quizesMap = new ConcurrentHashMap<String, Quiz>();    // todo
        quizCollector = new QuizCollector(quizesMap, dirListener);

        File file = new File(statusDir);
        if(!file.exists()) {
            file.mkdirs();
        }

      // todo load quizes
 }

    public void start() {
        scheduledDirListener.scheduleAtFixedRate(dirListener, listenerDelayFirst, listenerPeriod, java.util.concurrent.TimeUnit.SECONDS);
        scheduledQuizCollector.scheduleAtFixedRate(quizCollector,collectorDelayFirst,collectorPeriod,java.util.concurrent.TimeUnit.SECONDS);
    }

    public void stop() {
        scheduledDirListener.shutdown();
        scheduledQuizCollector.shutdown();
        try {
            replyStatsDataSource.shutdown();
        } catch (ReplyDataSourceException e) {
            logger.error("Error shutdown reply datasource", e);
        }
        subscriptionManager.shutdown();
    }

    public Result handleSms(String address, String oa, String text) throws QuizException{
        if(logger.isInfoEnabled()) {
            logger.info("Manager handle sms:" +address+" "+oa+" "+text);
        }
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

            quizBuilder.buildQuiz(fileName, null, quiz);
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
                makeSubscribedOnly(distribution);
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

    private void makeSubscribedOnly(Distribution distribution) throws QuizException{
        if((distribution==null)||(distribution.getFilePath()==null)) {
            return;
        }
        File file = new File(distribution.getFilePath());
        if(!file.exists()) {
            logger.error("Distributions abonents file doesn't exist");
            throw new QuizException("Distributions abonents file doesn't exist", QuizException.ErrorCode.ERROR_INIT);
        }
        String line = null;
        StringTokenizer tokenizer = null;
        PrintWriter writer = null;
        BufferedReader reader = null;
        String modifiedFileName = dirModifiedAb+"/"+file.getName()+".mod";
        try {
            reader = new BufferedReader(new FileReader(file));
            writer = new PrintWriter(new BufferedWriter(new FileWriter(modifiedFileName)));
            while((line = reader.readLine())!=null) {
                tokenizer = new StringTokenizer(line,"|");
                String msisdn = tokenizer.nextToken();
                if(subscriptionManager.subscribed(msisdn))  {   
                    writer.print(msisdn);
                    writer.print("|");
                    writer.println(tokenizer.nextToken());
                    if(logger.isInfoEnabled()) {
                        logger.info("Abonent subscribed: "+msisdn);
                    }
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
            if(writer!=null) {
                writer.close();
            }
            if(reader!=null) {
                try {
                    reader.close();
                } catch (IOException e) {
                    logger.error("Unabe to close file: ",e);
                }
            }
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

    public String getDirResult() {
        return dirResult;
    }

}
