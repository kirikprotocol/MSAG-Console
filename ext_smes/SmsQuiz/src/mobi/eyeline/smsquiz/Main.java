package mobi.eyeline.smsquiz;

import com.eyeline.sme.handler.MessageHandler;
import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.sme.smpp.ShutdownedException;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import com.sun.jdmk.comm.AuthInfo;
import com.sun.jdmk.comm.HtmlAdaptorServer;
import mobi.eyeline.smsquiz.distribution.impl.DistributionInfoSmeManager;
import mobi.eyeline.smsquiz.quizmanager.QuizManager;
import mobi.eyeline.smsquiz.replystats.datasource.impl.FileReplyStatsDataSource;
import mobi.eyeline.smsquiz.storage.ConnectionPoolFactory;
import mobi.eyeline.smsquiz.subscription.SubManagerException;
import mobi.eyeline.smsquiz.subscription.SubscriptionManager;
import org.apache.log4j.Logger;
import ru.aurorisoft.smpp.Message;

import javax.management.*;
import java.io.*;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;

/**
 * author: alkhal
 */
public class Main {

  private static final Logger logger = Logger.getLogger(Main.class);
  private static OutgoingQueue outgoingQueue;

  private static int quizIndex = 1;
  final static String conf = "conf/config.xml";
  private static ScheduledExecutorService scheduledQuizCreator;

  public static void main(String[] args) {
    //todo remove
    init();

    MessageHandler mh = null;
    QuizManager quizManager = null;
    try {

      ConnectionPoolFactory.init(conf);

      QuizManager.init(conf, new DistributionInfoSmeManager(conf),
          new FileReplyStatsDataSource(conf), SubscriptionManager.getInstance());
      quizManager = QuizManager.getInstance();

      PropertiesConfig cfg = new PropertiesConfig();
      cfg.load(new File("conf/smpp.properties"));
      final SMPPTransceiver transceiver = new SMPPTransceiver(cfg, "");
      outgoingQueue = transceiver.getOutQueue();
      transceiver.connect();
      mh = new MessageHandler(conf, transceiver.getInQueue(), transceiver.getOutQueue());
      mh.start();
      quizManager.start();

      final XmlConfig c = new XmlConfig();
      c.load(new File(conf));
      if (c.getSection("jmx") != null) {
        PropertiesConfig config = new PropertiesConfig(c.getSection("jmx").toProperties("."));
        int jmxPort = config.getInt("port");
        String user = config.getString("user");
        String password = config.getString("password");
        final MBeanServer mbs = quizManager.getMBeansServer();
        mbs.registerMBean(transceiver.getInQueueMonitor(), new ObjectName("SMSQUIZ.smpp:mbean=inQueue"));
        mbs.registerMBean(transceiver.getOutQueueMonitor(), new ObjectName("SMSQUIZ.smpp:mbean=outQueue"));
        mbs.registerMBean(mh.getHandlerMBean(), new ObjectName("SMSQUIZ.sme:mbean=handler"));
        registerAdapter(mbs, jmxPort, user, password);
      }

      //todo remove
      scheduledQuizCreator = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
        public Thread newThread(Runnable r) {
          return new Thread(r, "QuizCreator");
        }
      });
      //scheduledQuizCreator.scheduleAtFixedRate(new QuizCreator(), 10, 6000, TimeUnit.SECONDS);
      if((args!=null)&&(args.length>0)) {
        scheduledQuizCreator.scheduleAtFixedRate(new QuizCreator(), 10, 1500, TimeUnit.SECONDS);
      }
      //new QuizCreator().run();
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

  public static void registerAdapter(MBeanServer mBeanServer, int port, String user, String password) throws MBeanRegistrationException, InstanceAlreadyExistsException, NotCompliantMBeanException, MalformedObjectNameException {
    HtmlAdaptorServer adapter = new HtmlAdaptorServer(port, new AuthInfo[]{new AuthInfo(user, password)});
    mBeanServer.registerMBean(adapter, new ObjectName("SMSQUIZ:mbean=htmlAdaptor"));
    adapter.start();
  }

  //todo remove all

  private static class QuizCreator extends Thread {

 /*   public void run() {
      try {
        createAbFile("test_QuizManager/opros" + quizIndex + ".xml.csv", Long.parseLong("70001700000"), Long.parseLong("79131710000"), 500, SubscriptionManager.getInstance());
        createQuizFile("test_QuizManager/opros" + quizIndex + ".xml", 75, "170", "170", "Short\n question");
        quizIndex++;
        createAbFile("test_QuizManager/opros" + quizIndex + ".xml.csv", Long.parseLong("70001800000"), Long.parseLong("79131810000"), 500, SubscriptionManager.getInstance());
        createQuizFile("test_QuizManager/opros" + quizIndex + ".xml", 85, "180", "180", "Short\n question");
        quizIndex++;
        createAbFile("test_QuizManager/opros" + quizIndex + ".xml.csv", Long.parseLong("70001900000"), Long.parseLong("79131910000"), 500, SubscriptionManager.getInstance());
        createQuizFile("test_QuizManager/opros" + quizIndex + ".xml", 90, "190", "190", "Short\n question");
        quizIndex++;
      } catch (SubManagerException e) {
        logger.error("Error creatin quiz files", e);
        e.printStackTrace();
      }
    }  */

    public void run() {
      try {
        if(quizIndex<26) {
        createAbFile("test_QuizManager/opros" + quizIndex + ".xml.csv", Long.parseLong("70001700001"), Long.parseLong("70001700100"), 1, SubscriptionManager.getInstance());
        createQuizFile("test_QuizManager/opros" + quizIndex + ".xml", 20, "170", "170", "Short\n question");
        quizIndex++;
     /*   createAbFile("test_QuizManager/opros" + quizIndex + ".xml.csv", Long.parseLong("70001800001"), Long.parseLong("70001800100"), 1, SubscriptionManager.getInstance());
        createQuizFile("test_QuizManager/opros" + quizIndex + ".xml", 105, "180", "180", "Short\n question");
        quizIndex++;
        createAbFile("test_QuizManager/opros" + quizIndex + ".xml.csv", Long.parseLong("70001900001"), Long.parseLong("70001900100"), 1, SubscriptionManager.getInstance());
        createQuizFile("test_QuizManager/opros" + quizIndex + ".xml", 120, "190", "190", "Short\n question");
        quizIndex++;*/
        }
        else {
          scheduledQuizCreator.shutdown();
        }
      }
      catch (SubManagerException e) {
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
    Date dateBegin = new Date(cal.getTime().getTime()+180000);
    cal.set(Calendar.SECOND, 0);
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

  private static void createAbFile(String fileName, long from, long till, int divider, SubscriptionManager subscriptionManager) {


    PrintWriter writer = null;

    try {
      writer = new PrintWriter(new BufferedWriter(new FileWriter(fileName)));

      String abonent;

      for (long i = from; i <= till; i++) {
        abonent = "+" + i;
        writer.println(abonent);
        if ((i % divider) == 0) {
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
