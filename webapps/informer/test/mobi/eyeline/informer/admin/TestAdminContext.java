package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.blacklist.TestBlacklistManager;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.filesystem.TestFileSystem;
import mobi.eyeline.informer.admin.informer.TestInformerManager;
import mobi.eyeline.informer.admin.infosme.TestInfosme;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.regions.TestRegionsManager;
import mobi.eyeline.informer.admin.service.TestServiceManagerHA;
import mobi.eyeline.informer.admin.service.TestServiceManagerSingle;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.smsc.TestSmscManager;
import mobi.eyeline.informer.admin.users.TestUsersManager;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
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

  private void createDeliveries() throws AdminException{

    User u = null;
    {
      for (User user : usersManager.getUsers()) {
        if (user.hasRole("informer-admin")) {
          u = user;
          break;
        }
      }
    }
    if(u == null) {
      throw new IllegalArgumentException("User with 'admin' role is not found");
    }

    for(int i=1;i<=3;i++) {
      Delivery d = new Delivery();
      d.setActivePeriodEnd(new Date(System.currentTimeMillis() + 7*86400000*i));
      d.setActivePeriodStart(new Date(System.currentTimeMillis() - 7*86400000*i));
      d.setActiveWeekDays(new Delivery.Day[]{Delivery.Day.Fri, Delivery.Day.Sat});
      d.setDeliveryMode(DeliveryMode.SMS);
      d.setEndDate(new Date(System.currentTimeMillis() + 7*86400000*i));
      d.setName("Test delivery"+i);
      d.setOwner(u.getLogin());
      d.setPriority(15+i);
      d.setStartDate(new Date(System.currentTimeMillis() - 7*86400000*i));
      d.setSvcType("svc1");
      d.setValidityDate(new Date());

      deliveryManager.createDelivery(u.getLogin(),u.getPassword(), d, new MessageDataSource() {
        private LinkedList<Message> ms = new LinkedList<Message>() {
          {
            Random r = new Random();
            for(int k=0;k<100;k++) {
              Message m1 = Message.newTextMessage("text"+r.nextInt(10000));
              m1.setAbonent(new Address("+7913"+r.nextInt(10000)));
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
      }, null);

      assertNotNull(d.getId());
      deliveryManager.activateDelivery(u.getLogin(),u.getPassword(),d.getId());
    }
  }


  public TestAdminContext(File appBaseDir, WebConfig webConfig) throws InitException {
    fileSystem = new TestFileSystem();
    File servicesDir = new File(appBaseDir, "services");
    File confDir = new File(servicesDir, "Informer"+File.separatorChar+"conf");
    File statDir = new File(appBaseDir, "stat");
    servicesDir.mkdirs();
    confDir.mkdirs();
    try {
      prepareServices(confDir);
      prepareStat(statDir,fileSystem);

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

      deliveryManager = new TestDeliveryManager();
      createDeliveries();

      deliveryStatProvider = new TestDeliveryStatProvider(statDir, fileSystem);

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