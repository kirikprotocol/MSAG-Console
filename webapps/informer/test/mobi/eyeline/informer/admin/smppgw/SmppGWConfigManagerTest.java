package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.filesystem.MemoryFileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import org.junit.Before;
import org.junit.Test;

import java.io.BufferedWriter;
import java.io.File;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;

import static org.junit.Assert.assertEquals;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWConfigManagerTest {

  private SmppGWConfigManager manager;
  private FileSystem fs;

  @Before
  public void init() throws AdminException, InitException {
    fs = new MemoryFileSystem();
    prepareFiles(fs);
    initManager();
  }

  private void initManager() throws InitException {
    manager = new SmppGWConfigManager(new TestSmppGW(),new File("conf"), new File("conf"+File.separatorChar+"backup"), new SmppGWConfigManagerContext() {
      @Override
      public User getUser(String login) {
        User u =  new User();
        u.setLogin(login);
        u.setPassword(login+"_pwd");
        return u;
      }
      @Override
      public boolean containsDelivery(String login, int deliveryId) throws AdminException {
        return true;
      }
      @Override
      public FileSystem getFileSystem() {
        return fs;
      }
    });
  }

  private void prepareFiles(FileSystem fs) throws AdminException{
    fs.mkdirs(new File("conf"));
    PrintWriter w = null;
    try{
      w = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(new File("conf"+File.separatorChar+"deliveries.xml"), false))));
      w.print("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
          "\n" +
          "<config>\n" +
          "</config>");
    }finally {
      if(w != null) {
        w.close();
      }
    }
    w = null;
    try{
      w = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(new File("conf" + File.separatorChar + "endpoints.xml").getAbsoluteFile(), false))));
      w.print("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
          "\n" +
          "<config>\n" +
          "</config>");
    }finally {
      if(w != null) {
        w.close();
      }
    }
  }


  private static SmppGWRoute createSmppGWRoute() {
    SmppGWRoute route = new SmppGWRoute();
    route.setUser("admin");
    route.setDeliveryId(123);
    route.addServiceNumber(new Address(".5.0.MTC"));
    return route;
  }

  private static SmppGWProvider createProvider() throws AdminException {
    SmppGWProvider p = new SmppGWProvider();
    p.setName("provider");
    p.setDescr("descr");
    p.addEndpoint("endpoint1");
    p.addRoute(createSmppGWRoute());
    return p;
  }

  private static SmppGWEndpoint createEndpoint() {
    SmppGWEndpoint e = new SmppGWEndpoint();
    e.setName("endpoint1");
    e.setSystemId("systemid");
    e.setPassword("password");
    return e;
  }


  private static SmppGWEndpointSettings createESettings() throws AdminException {
    SmppGWEndpointSettings s = new SmppGWEndpointSettings();
    s.addEndpoint(createEndpoint());
    return s;
  }


  private static SmppGWProviderSettings createPSettings() throws AdminException {
    SmppGWProviderSettings s = new SmppGWProviderSettings();
    s.addProvider(createProvider());
    return s;
  }

  @Test
  public void loadSaveTest() throws AdminException, InitException {
    SmppGWProviderSettings ps = createPSettings();
    SmppGWEndpointSettings es = createESettings();

    manager.updateSettings(ps, es);

    initManager();

    SmppGWProviderSettings ps1 = manager.getProviderSettings();
    SmppGWEndpointSettings es1 = manager.getEndpointSettings();

    assertEquals(ps, ps1);
    assertEquals(es, es1);
  }
}
