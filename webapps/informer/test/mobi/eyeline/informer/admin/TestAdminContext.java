package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.archive.*;
import mobi.eyeline.informer.admin.blacklist.BlacklistManagerStub;
import mobi.eyeline.informer.admin.cdr.CdrProvider;
import mobi.eyeline.informer.admin.contentprovider.FileDeliveriesProvider;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.delivery.changelog.TestDeliveryChangesDetector;
import mobi.eyeline.informer.admin.delivery.stat.TestDeliveryStatProvider;
import mobi.eyeline.informer.admin.delivery.stat.TestUserStatProvider;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.filesystem.TestFileSystem;
import mobi.eyeline.informer.admin.informer.TestInformerManager;
import mobi.eyeline.informer.admin.infosme.TestInfosme;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.monitoring.InformerNotificationListener;
import mobi.eyeline.informer.admin.monitoring.MonitoringFileJournal;
import mobi.eyeline.informer.admin.notifications.DeliveryNotificationsProvider;
import mobi.eyeline.informer.admin.pvss.PVSSManager;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.regions.TestRegionsManager;
import mobi.eyeline.informer.admin.restriction.RestrictionProvider;
import mobi.eyeline.informer.admin.service.TestServiceManagerHA;
import mobi.eyeline.informer.admin.service.TestServiceManagerSingle;
import mobi.eyeline.informer.admin.siebel.SiebelProvider;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.smsc.TestSmscManager;
import mobi.eyeline.informer.admin.users.TestUsersManager;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;
import mobi.eyeline.informer.web.config.TimezonesConfig;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;
import java.util.Random;

import static org.junit.Assert.assertNotNull;

/**
 * Тестовый AdminContext
 * @author Aleksandr Khalitov
 */
public class TestAdminContext extends AdminContext {

  private void prepareServices(File confDir, File archiveConf) throws IOException, AdminException {
    TestUtils.exportResource(TestUsersManager.class.getResourceAsStream("users.xml"), new File(confDir,"users.xml"), false);
    TestUtils.exportResource(TestInformerManager.class.getResourceAsStream("config.xml"), new File(confDir, "config.xml"), false);
    TestUtils.exportResource(TestArchiveDaemonManager.class.getResourceAsStream("config.xml"), new File(archiveConf, "config.xml"), false);
    TestUtils.exportResource(TestSmscManager.class.getResourceAsStream("smsc.xml"), new File(confDir, "smsc.xml"), false);
    TestUtils.exportResource(TestRegionsManager.class.getResourceAsStream("regions.xml"), new File(confDir, "regions.xml"), false);
    TestUtils.exportResource(TimezonesConfig.class.getResourceAsStream("timezones.xml"), new File(confDir, "timezones.xml"), false);
   }

  private void prepareStat(File dstStatDir, FileSystem fileSystem) throws URISyntaxException, IOException, AdminException {
    if(!fileSystem.exists(dstStatDir)) {
      URL u = TestDeliveryStatProvider.class.getResource("");
      URI uri = u.toURI();

      if("jar".equals(uri.getScheme())) {
        String jarPath = uri.getSchemeSpecificPart();
        String jarFileURI   = jarPath.substring(0,jarPath.indexOf("!/"));
        String jarEntryPathURI = jarPath.substring(jarPath.indexOf("!/")+2,jarPath.length()-1);
        InputStream is = null;
        try {
          is = fileSystem.getInputStream(new File(new URI(jarFileURI)));
          TestUtils.extractDirFromJar(is,jarEntryPathURI,dstStatDir,fileSystem);
        }
        finally {
          if(is!=null) is.close();
        }

      }
      else {
        File srcStatDir = new File(uri);
//        srcStatDir = new File(srcStatDir,"stat");
        TestUtils.copyDirectory(srcStatDir,dstStatDir,fileSystem);
      }

    }
  }

  private void prepareNotificationLogs(File dstStatDir, FileSystem fileSystem) throws URISyntaxException, IOException, AdminException {
    if(!fileSystem.exists(dstStatDir)) {
      URL u = TestDeliveryChangesDetector.class.getResource("");
      URI uri = u.toURI();

      if("jar".equals(uri.getScheme())) {
        String jarPath = uri.getSchemeSpecificPart();
        String jarFileURI   = jarPath.substring(0,jarPath.indexOf("!/"));
        String jarEntryPathURI = jarPath.substring(jarPath.indexOf("!/")+2)+"changelog";
        InputStream is = null;
        try {
          is = fileSystem.getInputStream(new File(new URI(jarFileURI)));
          TestUtils.extractDirFromJar(is,jarEntryPathURI,dstStatDir,fileSystem);
        }
        finally {
          if(is!=null) is.close();
        }

      }
      else {
        File srcStatDir = new File(uri);
//        srcStatDir = new File(srcStatDir,"changelog");
        TestUtils.copyDirectory(srcStatDir,dstStatDir,fileSystem);
      }

    }
  }

  private void createDeliveries() throws AdminException {

    List<User> users = usersManager.getUsers();

    Random r1 = new Random();
    SimpleDateFormat df = new SimpleDateFormat("HHmmss");
    for(int i=1;i<=100;i++) {
      User u = users.get((i-1)%users.size());
      DeliveryPrototype d = new DeliveryPrototype();
      if(u.isCreateCDR()) {
        d.setEnableMsgFinalizationLogging(true);
      }
      d.setSourceAddress(new Address("+7901111"+i));
      d.setActivePeriodEnd(new Time(20,0,0));
      d.setActivePeriodStart(new Time(9,0,0));
      d.setActiveWeekDays(new Day[]{Day.Fri, Day.Mon, Day.Thu, Day.Wed, Day.Thu,});
      d.setDeliveryMode(DeliveryMode.SMS);
      if(i%5 == 1) {
        d.setEndDate(new Date(System.currentTimeMillis() + (r1.nextInt(6)+1)*86400000L*i));
      }
      d.setName("Test delivery"+i);
      d.setOwner(u.getLogin());
      d.setPriority(i%100 + 1);
      d.setStartDate(new Date(System.currentTimeMillis() - (r1.nextInt(6)+1)*86400000L*i));
      d.setSvcType("svc1");
      d.setValidityPeriod(new Time(1,0,0));

      Delivery deliv;
      if((i%2 == 1)) {
        deliv = deliveryManager.createDeliveryWithIndividualTexts(u.getLogin(),u.getPassword(), d, new DataSource<Message>() {
          private LinkedList<Message> ms = new LinkedList<Message>() {
            {
              Random r = new Random();
              for(int k=0;k<1000;k++) {
                Message m1 = Message.newMessage("text"+r.nextInt(10000));
                m1.setAbonent(new Address("+7913"+k));
                add(m1);
              }
            }
          };

          public Message next() throws AdminException {
            if(ms.isEmpty()) {
              return null;
            }
            return ms.removeFirst();
          }
        });
      }else {
        d.setSingleText("single text");
        deliv = deliveryManager.createDeliveryWithSingleText(u.getLogin(),u.getPassword(), d, new DataSource<Address>() {
          private LinkedList<Address> as = new LinkedList<Address>() {
            {
              for(int k=0;k<1000;k++) {
                add(new Address("+7913"+k));
              }
            }
          };

          public Address next() throws AdminException {
            if(as.isEmpty()) {
              return null;
            }
            return as.removeFirst();
          }
        });
      }

      assertNotNull(deliv.getId());
      deliveryManager.activateDelivery(u.getLogin(),u.getPassword(),deliv.getId());
    }
  }


  @SuppressWarnings({"ResultOfMethodCallIgnored"})
  public TestAdminContext(File appBaseDir) throws InitException {
    try {
      File webConfDir = new File(appBaseDir,"conf");
      TestUtils.exportResource(TestWebConfigManager.class.getResourceAsStream("webconfig.xml"), new File(webConfDir, "webconfig.xml"), false);
      this.webConfig = new  TestWebConfigManager(new File(webConfDir,"webconfig.xml") ,new File(webConfDir, "backup"), FileSystem.getFSForSingleInst()) ;

      fileSystem = new TestFileSystem();


      File monitoringDir = new File(webConfig.getMonitoringDir());
      if(!monitoringDir.exists() && !monitoringDir.mkdirs()) {
        throw new InitException("Can't create dir: "+monitoringDir.getAbsolutePath());
      }

      monitoringJournal = new MonitoringFileJournal(monitoringDir, fileSystem);
      InformerNotificationListener.setJournal(monitoringJournal);

      File servicesDir = new File(appBaseDir, "services");
      File confDir = new File(servicesDir, "Informer"+File.separatorChar+"conf");
      File archiveDaemonConf = new File(servicesDir, "ArchiveDaemon"+File.separatorChar+"conf");
      File statDir = new File(appBaseDir, "stat");
      File statusLogsDir = new File(appBaseDir, "statuslogs");

      workDir = new File(webConfig.getWorkDir());
      
      if(!workDir.exists() && !workDir.mkdirs()) {
        throw new InitException("Can't create work dir: "+workDir.getAbsolutePath());
      }
      servicesDir.mkdirs();
      confDir.mkdirs();
      archiveDaemonConf.mkdirs();

      prepareServices(confDir, archiveDaemonConf);
      prepareStat(statDir,fileSystem);
      prepareNotificationLogs(statusLogsDir,fileSystem);

      if (webConfig.getInstallationType() == InstallationType.SINGLE)  {
        serviceManager = new TestServiceManagerSingle(servicesDir);
      }else {
        serviceManager = new TestServiceManagerHA(servicesDir, new String[] {"host0", "host1", "host2"});
      }

      journal = new Journal(new File(webConfig.getJournalDir()), fileSystem);
      informerManager = new TestInformerManager(new File(confDir, "config.xml"),
          new File(confDir, "backup"), fileSystem, serviceManager);
      archiveDaemonManager = new TestArchiveDaemonManager(new File(archiveDaemonConf, "config.xml"),
          new File(confDir, "backup"), fileSystem, serviceManager);
      pvssManager = new PVSSManager(serviceManager);
      infosme = new TestInfosme();
      usersManager = new TestUsersManager(infosme, new File(confDir, "users.xml"),new File(confDir, "backup"), fileSystem);

      blacklistManager = new BlacklistManagerStub();
      smscManager = new TestSmscManager(infosme, new File(confDir, "smsc.xml"),
          new File(confDir, "backup"), fileSystem);
      for(Smsc s : smscManager.getSmscs()) {
        infosme.addSmsc(s.getName());
      }
      regionsManager = new TestRegionsManager(infosme, new File(confDir, "regions.xml"),
          new File(confDir, "backup"), fileSystem);
      for(Region s : regionsManager.getRegions()) {
        infosme.addRegion(s.getRegionId());
      }

      deliveryManager = new TestDeliveryManager(workDir, fileSystem);
      statisticsManager = new TestStatisticsManager(new TestDeliveryStatProvider(statDir, fileSystem), new TestUserStatProvider(statDir, fileSystem));
      createDeliveries();

      restrictionProvider = new RestrictionProvider(this, new File(confDir, "restrictions.csv"), new File(confDir, "backup"), fileSystem);

      deliveryChangesDetector = new TestDeliveryChangesDetector(statusLogsDir,fileSystem);
      deliveryNotificationsProvider = new DeliveryNotificationsProvider(this, webConfig.getNotificationSettings());
      deliveryChangesDetector.addListener(deliveryNotificationsProvider);

      fileDeliveriesProvider = new FileDeliveriesProvider(this, appBaseDir, workDir);

      siebelProvider = new SiebelProvider(this, webConfig.getSiebelSettings(), workDir);

      cdrProvider = new CdrProvider(this, webConfig.getCdrSettings(), new File(workDir, "cdr"), fileSystem);

      deliveryChangesDetector.start();

      ArchiveSettings settings = webConfig.getArchiveSettings();
      settings.setRequestsDir(new File(workDir, "archive_requests").getAbsolutePath());
      settings.setResultsDir(new File(workDir, "archive_results").getAbsolutePath());

      archiveDeliveryManager = deliveryManager;
      archiveRequestsManager = new ArchiveRequestsManager(this, settings);
      User u = usersManager.getUsers().get(0);
      archiveRequestsManager.createRequest(u.getLogin(), createDeliveriesPrototype("deliveries 1 "+System.currentTimeMillis()));
      archiveRequestsManager.createRequest(u.getLogin(), createDeliveriesPrototype("deliveries 2 "+System.currentTimeMillis()));
      archiveRequestsManager.createRequest(u.getLogin(), createMessagesPrototype("messages 1 "+System.currentTimeMillis(), new Date(0), "+7913182"));
      archiveRequestsManager.createRequest(u.getLogin(), createMessagesPrototype("messages 2 "+System.currentTimeMillis(), new Date(0), "+7913178"));

      informerConfDir = confDir;

    } catch (IOException e) {
      throw new InitException(e);
    }catch (AdminException e) {
      throw new InitException(e);
    }
    catch (URISyntaxException e) {
      throw new InitException(e);
    }

    initDependencies();
  }



  private static DeliveriesRequestPrototype createDeliveriesPrototype(String name) {
    DeliveriesRequestPrototype q = new DeliveriesRequestPrototype();
    q.setName(name);
    q.setFrom(new Date(0));
    return q;
  }


  private static MessagesRequestPrototype createMessagesPrototype(String name, Date from, String address) {
    MessagesRequestPrototype q = new MessagesRequestPrototype();
    q.setName(name);
    q.setFrom(from);
    q.setAddress(new Address(address));
    return q;
  }

}