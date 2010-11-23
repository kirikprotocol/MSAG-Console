package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.blacklist.TestBlacklistManager;
import mobi.eyeline.informer.admin.contentprovider.ContentProviderDaemon;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.filesystem.TestFileSystem;
import mobi.eyeline.informer.admin.informer.TestInformerManager;
import mobi.eyeline.informer.admin.infosme.TestInfosme;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.notifications.DeliveryNotificationsDaemon;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.regions.TestRegionsManager;
import mobi.eyeline.informer.admin.restriction.RestrictionDaemon;
import mobi.eyeline.informer.admin.restriction.TestRestrictionsManager;
import mobi.eyeline.informer.admin.service.TestServiceManagerHA;
import mobi.eyeline.informer.admin.service.TestServiceManagerSingle;
import mobi.eyeline.informer.admin.siebel.SiebelManager;
import mobi.eyeline.informer.admin.siebel.TestSiebelManager;
import mobi.eyeline.informer.admin.siebel.impl.SiebelDeliveries;
import mobi.eyeline.informer.admin.siebel.impl.SiebelFinalStateListener;
import mobi.eyeline.informer.admin.siebel.impl.SiebelRegionManager;
import mobi.eyeline.informer.admin.siebel.impl.SiebelUserManager;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.smsc.TestSmscManager;
import mobi.eyeline.informer.admin.users.TestUsersManager;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;
import mobi.eyeline.informer.web.TestWebConfigManager;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.*;

import static org.junit.Assert.assertNotNull;

/**
 * Тестовый AdminContext
 * @author Aleksandr Khalitov
 */
public class TestAdminContext extends AdminContext {

  private void prepareServices(File confDir) throws IOException, AdminException {
    TestUtils.exportResource(TestUsersManager.class.getResourceAsStream("users.xml"), new File(confDir,"users.xml"), false);
    TestUtils.exportResource(TestInformerManager.class.getResourceAsStream("config.xml"), new File(confDir, "config.xml"), false);
    TestUtils.exportResource(TestSmscManager.class.getResourceAsStream("smsc.xml"), new File(confDir, "smsc.xml"), false);
    TestUtils.exportResource(TestRegionsManager.class.getResourceAsStream("regions.xml"), new File(confDir, "regions.xml"), false);
    TestUtils.exportResource(TestRestrictionsManager.class.getResourceAsStream("restrictions.csv"), new File(confDir, "restrictions.csv"), false);
  }

  private void prepareStat(File dstStatDir, FileSystem fileSystem) throws URISyntaxException, IOException, AdminException {
    if(!fileSystem.exists(dstStatDir)) {
      URL u = TestDeliveryStatProvider.class.getResource("");
      URI uri = u.toURI();

      if("jar".equals(uri.getScheme())) {
        String jarPath = uri.getSchemeSpecificPart();
        String jarFileURI   = jarPath.substring(0,jarPath.indexOf("!/"));
        String jarEntryPathURI = jarPath.substring(jarPath.indexOf("!/")+2)+"stat";
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
        srcStatDir = new File(srcStatDir,"stat");
        TestUtils.copyDirectory(srcStatDir,dstStatDir,fileSystem);
      }

    }
  }

  private void prepareNotificationLogs(File dstStatDir, FileSystem fileSystem) throws URISyntaxException, IOException, AdminException {
    if(!fileSystem.exists(dstStatDir)) {
      URL u = TestDeliveryStatProvider.class.getResource("");
      URI uri = u.toURI();

      if("jar".equals(uri.getScheme())) {
        String jarPath = uri.getSchemeSpecificPart();
        String jarFileURI   = jarPath.substring(0,jarPath.indexOf("!/"));
        String jarEntryPathURI = jarPath.substring(jarPath.indexOf("!/")+2)+"statuslogs";
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
        srcStatDir = new File(srcStatDir,"statuslogs");
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
      Delivery d = (i%2 == 1) ? Delivery.newCommonDelivery() : Delivery.newSingleTextDelivery();
      d.setSourceAddress(new Address("+7901111"+i));
      d.setActivePeriodEnd(new Time(20,0,0));
      d.setActivePeriodStart(new Time(9,0,0));
      d.setActiveWeekDays(new Delivery.Day[]{Delivery.Day.Fri, Delivery.Day.Mon, Delivery.Day.Thu, Delivery.Day.Wed, Delivery.Day.Thu,});
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
      if(d.getType() == Delivery.Type.Common) {
        deliveryManager.createDelivery(u.getLogin(),u.getPassword(), d, new DataSource<Message>() {
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
        deliveryManager.createSingleTextDelivery(u.getLogin(),u.getPassword(), d, new DataSource<Address>() {
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

      assertNotNull(d.getId());
      deliveryManager.activateDelivery(u.getLogin(),u.getPassword(),d.getId());
    }
  }


  @SuppressWarnings({"ResultOfMethodCallIgnored"})
  public TestAdminContext(File appBaseDir) throws InitException {

    try {
      File webConfDir = new File(appBaseDir,"conf");
      TestUtils.exportResource(TestWebConfigManager.class.getResourceAsStream("webconfig.xml"), new File(webConfDir, "webconfig.xml"), false);
      this.webConfig = new  TestWebConfigManager(new File(webConfDir,"webconfig.xml") ,new File(webConfDir, "backup"), FileSystem.getFSForSingleInst()) ;

      fileSystem = new TestFileSystem();
      File servicesDir = new File(appBaseDir, "services");
      File confDir = new File(servicesDir, "Informer"+File.separatorChar+"conf");
      File statDir = new File(appBaseDir, "stat");
      File statusLogsDir = new File(appBaseDir, "statuslogs");
      workDir = new File(appBaseDir, "work");
      if(!workDir.exists() && !workDir.mkdirs()) {
        throw new InitException("Can't create work dir: "+workDir.getAbsolutePath());
      }
      servicesDir.mkdirs();
      confDir.mkdirs();

      prepareServices(confDir);
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
      infosme = new TestInfosme();
      usersManager = new TestUsersManager(infosme, new File(confDir, "users.xml"),new File(confDir, "backup"), fileSystem);

      blacklistManager = new TestBlacklistManager();
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

      deliveryManager = new TestDeliveryManager(new TestDeliveryStatProvider(statDir, fileSystem));
      createDeliveries();

      restrictionsManager = new TestRestrictionsManager(infosme, new File(confDir, "restrictions.csv"),
          new File(confDir, "backup"), fileSystem);

      restrictionDaemon = new RestrictionDaemon(deliveryManager,restrictionsManager,usersManager);
      contentProviderDaemon = new ContentProviderDaemon(this,appBaseDir, workDir, fileSystem);

      deliveryNotificationsProducer  = new TestDeliveryNotificationsProducer(statusLogsDir,fileSystem);
      deliveryNotificationsDaemon    = new DeliveryNotificationsDaemon(this);
      deliveryNotificationsProducer.addListener(deliveryNotificationsDaemon);

      try{
        initSiebel(workDir);
      }catch (AdminException e){
        logger.error(e,e);
      }
      
      deliveryNotificationsProducer.start();

    } catch (IOException e) {
      throw new InitException(e);
    }catch (AdminException e) {
      throw new InitException(e);
    }
    catch (URISyntaxException e) {
      throw new InitException(e);
    }
  }


  private static final List<Daemon> testDaemons = new ArrayList<Daemon>(3) {{
    for(int i =0; i<3; i++) {
      add(new TestDaemon(i));
    }
  }};

  @Override
  protected void initSiebel(File workFile) throws AdminException, InitException {

    SiebelDeliveries siebelDeliveries = new SiebelDeliveriesImpl(this);
    SiebelRegionManager siebelRegions = new SiebelRegionManagerImpl(this);
    SiebelUserManager userManager = new SiebelUserManagerImpl(this);

    User siebelUser = usersManager.getUser(webConfig.getSiebelProperties().getProperty(SiebelManager.USER));

    if(siebelUser == null) {
      throw new IntegrityException("user_not_exist", webConfig.getSiebelProperties().getProperty(SiebelManager.USER));
    }

    siebelManager = new TestSiebelManager(siebelDeliveries, siebelRegions);

    siebelManager.start(siebelUser, webConfig.getSiebelProperties());

    siebelFinalStateListener = new SiebelFinalStateListener(siebelManager, siebelDeliveries, userManager, workFile, 20);  //todo

    deliveryNotificationsProducer.addListener(siebelFinalStateListener);

  }

  @Override
  public List<Daemon> getDaemons() {
    List<Daemon> ds = super.getDaemons();
    ds.addAll(testDaemons);
    return ds;
  }


  private static class TestDaemon implements Daemon {

    private boolean isStarted = false;

    private String name;

    private TestDaemon(int index ) {
      name = "TestDaemon"+index;
    }

    public String getName() {
      return name;
    }

    public void start() throws AdminException {
      isStarted = true;
    }

    public void stop() throws AdminException {
      isStarted = false;
    }

    public boolean isStarted() {
      return isStarted;
    }
  }
}