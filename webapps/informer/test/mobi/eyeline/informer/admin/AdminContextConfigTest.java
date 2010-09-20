package mobi.eyeline.informer.admin;

import org.junit.BeforeClass;
import org.junit.Test;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

import static org.junit.Assert.assertEquals;

/**
 * @author Aleksandr Khalitov
 */
public class AdminContextConfigTest {


  private static AdminContextConfig config;

  @BeforeClass
  public static void init() throws Exception{
    InputStream is = null;
    try{
      is = AdminContextConfigTest.class.getResourceAsStream("webconfig.xml");
      config = new AdminContextConfig(is);
    }finally {
      if(is != null) {
        try{
          is.close();
        }catch (IOException e){}
      }
    }
  }

  @Test(expected = IllegalArgumentException.class)
  public void testNull1() throws AdminException{
    InputStream is = null;
    new AdminContextConfig(is);
  }
  
  @Test(expected = IllegalArgumentException.class)
  public void testNull2() throws AdminException{
    File f = null;
    new AdminContextConfig(f);
  }

  @Test
  public void testData() throws Exception{
    assertEquals(config.getUsersFile(), "test/ru/novosoft/smsc/admin/users/users.xml");
    assertEquals(config.getInstallationType(), InstallationType.HA);
    assertEquals(config.getHSDaemonHost(), "localhost");
    assertEquals(config.getSingleDaemonHost(), "localhost");
    assertEquals(config.getHSDaemonPort(), 10000);
    assertEquals(config.getSingleDaemonPort(), 10000);
    assertEquals(config.getAppMirrorDirs().length, 1);
    assertEquals(config.getAppMirrorDirs()[0].getAbsoluteFile(), new File("mirror").getAbsoluteFile());
    assertEquals(config.getHSDaemonHosts().size(), 1);
    assertEquals(config.getHSDaemonHosts().iterator().next(),"sunfire");
  }

}
