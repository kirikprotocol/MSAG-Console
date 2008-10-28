package mobi.eyeline.smsquiz;

import com.eyeline.sme.handler.MessageHandler;
import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sme.smpp.ShutdownedException;
import com.eyeline.utils.config.properties.PropertiesConfig;

import java.io.*;
import java.util.Calendar;
import java.util.Date;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
import java.text.SimpleDateFormat;

import mobi.eyeline.smsquiz.storage.ConnectionPoolFactory;
import mobi.eyeline.smsquiz.distribution.impl.DistributionInfoSmeManager;
import mobi.eyeline.smsquiz.replystats.datasource.impl.FileReplyStatsDataSource;
import mobi.eyeline.smsquiz.subscription.SubscriptionManager;
import mobi.eyeline.smsquiz.subscription.SubManagerException;
import mobi.eyeline.smsquiz.quizmanager.QuizManager;
import org.apache.log4j.Logger;
import ru.aurorisoft.smpp.Message;

/**
 * author: alkhal
 */
public class Main {

  private static final Logger logger = Logger.getLogger(Main.class);
  private static OutgoingQueue outgoingQueue;

  private static int quizIndex = 1;
  private static ScheduledExecutorService scheduledQuizCreator;

  public static void main(String[] args) {
    //todo remove
    init();
    final String conf = "conf/config.xml";
    MessageHandler mh = null;
    QuizManager quizManager = null;
    try {
      ConnectionPoolFactory.init("conf/config.xml");

      QuizManager.init(conf, new DistributionInfoSmeManager(conf),
          new FileReplyStatsDataSource("conf/config.xml"), SubscriptionManager.getInstance());
      quizManager = QuizManager.getInstance();

      PropertiesConfig cfg = new PropertiesConfig();
      cfg.load(new File("conf/smpp.properties"));
      final SMPPTransceiver transceiver = new SMPPTransceiver(cfg, "");
      outgoingQueue = transceiver.getOutQueue();
      transceiver.connect();
      mh = new MessageHandler(conf, transceiver.getInQueue(), transceiver.getOutQueue());
      mh.start();
      quizManager.start();

      //todo remove
      scheduledQuizCreator = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
        public Thread newThread(Runnable r) {
          return new Thread(r, "QuizCreator");
        }
      });
      scheduledQuizCreator.scheduleAtFixedRate(new QuizCreator(), 10, 5400, TimeUnit.SECONDS);
      //todo remove

    } catch (Exception e) {
      logger.error("", e);
      e.printStackTrace();
      if (quizManager != null) {
        quizManager.stop();
      }
      if (mh != null) {
        try {
          mh.shutdown();
        } catch (InterruptedException e1) {
          logger.error("Error during stoping sme", e1);
          e.printStackTrace();
        }
      }
    }
  }

  //todo remove all

  private static class QuizCreator extends Thread {
    public void run() {
      try {
        createAbFile("test_QuizManager/opros" + quizIndex + ".xml.csv", 791366, 791367, SubscriptionManager.getInstance());
        createQuizFile("test_QuizManager/opros" + quizIndex + ".xml", 60, "170", "170", "Short\n question");
        quizIndex++;
        createAbFile("test_QuizManager/opros" + quizIndex + ".xml.csv", 791367, 791368, SubscriptionManager.getInstance());
        createQuizFile("test_QuizManager/opros" + quizIndex + ".xml", 70, "180", "180", "Medium question");
        quizIndex++;
        createAbFile("test_QuizManager/opros" + quizIndex + ".xml.csv", 791368, 791369, SubscriptionManager.getInstance());
        createQuizFile("test_QuizManager/opros" + quizIndex + ".xml", 80, "190", "190", "Long question");
        quizIndex++;
      } catch (SubManagerException e) {
        logger.error("Error creatin quiz files", e);
        e.printStackTrace();
      }
    }
  }

  public static int send(String da, String oa, String text) {
    if (outgoingQueue == null) {
      logger.error("Error during sending message, outgoingQueue==null");
      return 1;
    }
    OutgoingObject outObj = new OutgoingObject();
    Message m = new Message();
    m.setDestinationAddress(da);
    m.setSourceAddress(oa);
    m.setMessageString(text);
    outObj.setMessage(m);
    try {
      outgoingQueue.offer(outObj, 3000);
      if (logger.isInfoEnabled()) {
        logger.info("Message send to abonent: " + da + " " + oa + " " + text);
      }
      return 0;
    } catch (ShutdownedException e) {
      logger.error("Error during sending message", e);
      e.printStackTrace();
      return 1;
    }
  }


  private static void init() {
    File file = new File("test_QuizManager");
    if (file.exists()) {
      removeAll(file);
    }
    file.mkdirs();
    file = new File("replyStats");
    if (file.exists()) {
      removeAll(file);
    }
    file = new File("quizResults");
    if (file.exists()) {
      removeAll(file);
    }

  }

  private static void createQuizFile(String fileName, int minutes, String oa, String da, String question) {

    SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm");
    Calendar cal = Calendar.getInstance();
    cal.setTimeInMillis(System.currentTimeMillis());
    cal.set(Calendar.SECOND, 0);
    Date dateBegin = cal.getTime();
    cal.add(Calendar.MINUTE, minutes);
    Date dateEnd = cal.getTime();

    PrintWriter writer = null;
    try {
      writer = new PrintWriter(new BufferedWriter(new FileWriter(fileName)));
      writer.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
      writer.println("<opros>");

      writer.println("    <general>");
      writer.print("        <date-begin>");
      writer.print(dateFormat.format(dateBegin));
      writer.println("</date-begin>");
      writer.print("        <date-end>");
      writer.print(dateFormat.format(dateEnd));
      writer.println("</date-end>");
      writer.print("        <question>");
      writer.println(question);
      writer.println("</question>");
      writer.print("        <abonents-file>");
      writer.print(fileName);
      writer.print(".csv");
      writer.println("</abonents-file>");
      writer.println("    </general>");

      writer.println("    <distribution>");
      writer.print("        <source-address>");
      writer.print(oa);
      writer.println("</source-address>");
      writer.println("        <time-begin>00:01</time-begin>");
      writer.println("        <time-end>23:59</time-end>");
      writer.println("        <days>");         //Mon, Tue, Wed, Thu, Fri, Sat, Sun
      writer.println("            <day>Mon</day>");
      writer.println("            <day>Sun</day>");
      writer.println("            <day>Tue</day>");
      writer.println("            <day>Wed</day>");
      writer.println("            <day>Thu</day>");
      writer.println("            <day>Fri</day>");
      writer.println("            <day>Sat</day>");
      writer.println("        </days>");
      writer.println("        <txmode>transaction</txmode>");
      writer.println("    </distribution>");

      writer.println("    <replies>");
      writer.print("        <destination-address>");
      writer.print(da);
      writer.println("</destination-address>");
      writer.println("        <max-repeat>3</max-repeat>");

      writer.println("        <reply>");
      writer.println("            <category>Да</category>");
      writer.println("            <pattern>(yes|y|1|да|д)</pattern>");
      writer.println("            <answer>Thanks</answer>");
      writer.println("        </reply>");

      writer.println("        <reply>");
      writer.println("            <category>Нет</category>");
      writer.println("            <pattern>(no|n|0|нет|н)</pattern>");
      writer.println("            <answer>Thanks</answer>");
      writer.println("        </reply>");

      writer.println("        <default>Да</default>");

      writer.println("    </replies>");
      writer.println("</opros>");
      writer.flush();
    } catch (IOException e) {
      e.printStackTrace();
    } finally {
      if (writer != null) {
        writer.close();
      }
    }
  }

  private static void createAbFile(String fileName, long from, long till, SubscriptionManager subscriptionManager) {
    from *= 100000;
    till *= 100000;

    PrintWriter writer = null;

    try {
      writer = new PrintWriter(new BufferedWriter(new FileWriter(fileName)));

      String abonent;

      for (long i = from; i < till; i++) {
        abonent = "+" + i;
        writer.println(abonent);
        if ((i % 500) == 0) {
          try {
            subscriptionManager.subscribe(abonent);
          } catch (SubManagerException e) {
            e.printStackTrace();
          }
        }
      }
      writer.flush();
    } catch (IOException e) {
      e.printStackTrace();
    } finally {
      if (writer != null) {
        writer.close();
      }
    }
  }

  private static void removeAll(File dir) {
    if ((dir == null) || (!dir.exists())) {
      return;
    }
    if (dir.isDirectory()) {
      for (File f : dir.listFiles()) {
        removeAll(f);
      }
    }
    dir.delete();
  }

}
