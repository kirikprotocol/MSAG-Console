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

  private static final Logger logger = Logger.getLogger(MainTest.class);
  private static OutgoingQueue outgoingQueue;

  private static int quizIndex = 1;
  final static String conf = "conf/config.xml";
  private static ScheduledExecutorService scheduledQuizCreator;

  public static void main(String[] args) {

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

}
