package mobi.eyeline.smsquiz;

import com.eyeline.sme.handler.MessageHandler;
import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import com.sun.jdmk.comm.AuthInfo;
import com.sun.jdmk.comm.HtmlAdaptorServer;
import mobi.eyeline.smsquiz.distribution.impl.DistributionInfoSmeManager;
import mobi.eyeline.smsquiz.quizmanager.QuizManager;
import mobi.eyeline.smsquiz.quizmanager.SmsQuizService;
import mobi.eyeline.smsquiz.replystats.datasource.impl.FileReplyStatsDataSource;
import mobi.eyeline.smsquiz.storage.ConnectionPoolFactory;
import mobi.eyeline.smsquiz.subscription.SubscriptionManager;
import org.apache.log4j.Logger;
import ru.sibinco.smsc.utils.admin.server.CommandServer;

import javax.management.*;
import java.io.File;

/**
 * author: alkhal
 */
public class Main {

  private static final Logger logger = Logger.getLogger(Main.class);

  final static String conf = "conf/config.xml";

  private static void startCommandServer(XmlConfig c, QuizManager manager) {
    CommandServer server = null;
    try {
      if (manager == null) {
        logger.error("QuizManager is crushed, Command server will not started");
        return;
      }
      PropertiesConfig config = new PropertiesConfig(c.getSection("commandserver").toProperties("."));
      int port = config.getInt("port");
      server = new CommandServer(port, 5, 30, 120, 120);
      server.start();

      SmsQuizService service = new SmsQuizService(manager);
      server.registerService(service, service.getDescription());
    }
    catch (Exception e) {
      if (server != null) {
        server.shutdown();
      }
    }
  }

  private static QuizManager startSmsQuiz(XmlConfig c) {
    MessageHandler mh = null;
    QuizManager quizManager = null;
    try {
      ConnectionPoolFactory.init(conf);
      PropertiesConfig cfg = new PropertiesConfig(c.getSection("quizmanager").toProperties("."));

      QuizManager.init(conf, new DistributionInfoSmeManager(conf, cfg.getString("dir_work")),
          new FileReplyStatsDataSource(conf), SubscriptionManager.getInstance());
      quizManager = QuizManager.getInstance();

      cfg = new PropertiesConfig();
      cfg.load(new File("conf/smpp.properties"));
      final SMPPTransceiver transceiver = new SMPPTransceiver(cfg, "");
      transceiver.connect();
      mh = new MessageHandler(conf, transceiver.getInQueue(), transceiver.getOutQueue());
      mh.start();
      quizManager.start();

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
    }
    catch (Exception e) {
      logger.error(e, e);
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
    return quizManager;
  }


  public static void main(String[] args) {
    try {
      final XmlConfig c = new XmlConfig();
      c.load(new File(conf));
      QuizManager manager = startSmsQuiz(c);
      startCommandServer(c, manager);
    } catch (Exception e) {
      logger.error(e, e);
    }
  }

  private static void registerAdapter(MBeanServer mBeanServer, int port, String user, String password) throws MBeanRegistrationException, InstanceAlreadyExistsException, NotCompliantMBeanException, MalformedObjectNameException {
    HtmlAdaptorServer adapter = new HtmlAdaptorServer(port, new AuthInfo[]{new AuthInfo(user, password)});
    mBeanServer.registerMBean(adapter, new ObjectName("SMSQUIZ:mbean=htmlAdaptor"));
    adapter.start();
  }

}
