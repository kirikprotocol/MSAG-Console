package mobi.eyeline.smsquiz;

import com.eyeline.sme.handler.MessageHandler;
import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.utils.config.properties.PropertiesConfig;

import java.io.*;
import java.util.Calendar;
import java.util.Date;
import java.text.SimpleDateFormat;

import mobi.eyeline.smsquiz.storage.ConnectionPoolFactory;
import mobi.eyeline.smsquiz.distribution.Impl.DistributionInfoSmeManager;
import mobi.eyeline.smsquiz.replystats.datasource.impl.FileReplyStatsDataSource;
import mobi.eyeline.smsquiz.subscription.SubscriptionManager;
import mobi.eyeline.smsquiz.subscription.SubManagerException;
import mobi.eyeline.smsquiz.quizmanager.QuizManager;
import org.apache.log4j.Logger;

/**
 * author: alkhal
 */
public class Main {

  private static final Logger logger = Logger.getLogger(Main.class);

  public static void main(String[] args){
    final String conf="conf/config.xml";
    MessageHandler mh = null;
    QuizManager quizManager = null;
    try{
      ConnectionPoolFactory.init("conf/config.xml");

       //todo remove
      init(SubscriptionManager.getInstance());
      createQuizFile("test_QuizManager/short.xml", 2, "170", "170","Short question");
      createAbFile("test_QuizManager/short.xml.csv", 79136666, 79136667, SubscriptionManager.getInstance());
       //todo remove

      QuizManager.init(conf, new DistributionInfoSmeManager(conf),
          new FileReplyStatsDataSource("conf/config.xml"), SubscriptionManager.getInstance());
      quizManager = QuizManager.getInstance();

      PropertiesConfig cfg = new PropertiesConfig();
      cfg.load(new File("conf/smpp.properties"));
      final SMPPTransceiver transceiver = new SMPPTransceiver(cfg, "");
      transceiver.connect();
      mh = new MessageHandler(conf,transceiver.getInQueue(),transceiver.getOutQueue());
      mh.start();
      quizManager.start();


       //todo remove
      createQuizFile("test_QuizManager/medium.xml", 3, "180", "180", "Medium question");
      createAbFile("test_QuizManager/medium.xml.csv", 79136666, 79136668, SubscriptionManager.getInstance());
      createQuizFile("test_QuizManager/long.xml", 4, "190", "190", "Long question");
      createAbFile("test_QuizManager/long.xml.csv", 79136666, 79136669, SubscriptionManager.getInstance());
       //todo remove

    } catch (Exception e) {
      logger.error("",e);
      e.printStackTrace();
      if(quizManager!=null) {
        quizManager.stop();
      }
      if(mh!=null) {
        try {
          mh.shutdown();
        } catch (InterruptedException e1) {
          logger.error("Error during stoping sme",e1);
          e.printStackTrace();
        }
      }
    }
  }

  //todo remove
  private static void init(SubscriptionManager subscriptionManager) {
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

  //todo remove
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
      writer.print("        <question>");writer.println(question);writer.println("</question>");
      writer.print("        <abonents-file>");writer.print(fileName);writer.print(".csv");writer.println("</abonents-file>");
      writer.println("    </general>");

      writer.println("    <distribution>");
      writer.print("        <source-address>");
      writer.print(oa);
      writer.println("</source-address>");
      writer.println("        <time-begin>12:00</time-begin>");
      writer.println("        <time-end>20:00</time-end>");
      writer.println("        <days>");
      writer.println("            <day>Mon</day>");
      writer.println("            <day>Sun</day>");
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

  //todo remove
  private static void createAbFile(String fileName, long from, long till, SubscriptionManager subscriptionManager) {
    from*=1000;
    till*=1000;

    PrintWriter writer = null;

    try {
      writer = new PrintWriter(new BufferedWriter(new FileWriter(fileName)));
      try {
        subscriptionManager.subscribe("7909");
        subscriptionManager.subscribe("7910");
      } catch (SubManagerException e) {
        e.printStackTrace();
      }
      String abonent = "7909";
      writer.print(abonent);
      writer.print("|");

      StringBuilder strBuilder = new StringBuilder(30);
      int aCode = (int) 'a';
      for (int j = 0; j < 20; j++) {
        strBuilder.append((char) (aCode + 26 * Math.random()));
      }
      writer.println(strBuilder.substring(0));
      abonent = "7910";
      writer.print(abonent);
      writer.print("|");

      strBuilder = new StringBuilder(20);
      for (int j = 0; j < 20; j++) {
        strBuilder.append((char) (aCode + 26 * Math.random()));
      }
      writer.println(strBuilder.substring(0));
      for (long i = from; i < till; i++) {
        abonent = "+" + i;
        writer.print(abonent);
        writer.print("|");
        if ((i % 100) == 0) {
          try {
            subscriptionManager.subscribe(abonent);
          } catch (SubManagerException e) {
            e.printStackTrace();
          }
        }
        strBuilder = new StringBuilder(20);
        aCode = (int) 'a';
        for (int j = 0; j < 20; j++) {
          strBuilder.append((char) (aCode + 26 * Math.random()));
        }
        writer.println(strBuilder.substring(0));
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
  //todo remove
  private static void removeAll(File dir) {
    if ((dir == null) || (!dir.exists())) {
      return;
    }
    if (dir.isDirectory()) {
      for (File f : dir.listFiles()) {
        removeAll(f);
      }
    }
    if (dir.isFile()) {
      dir.delete();
    }
  }

}
