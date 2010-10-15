package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.InstallationType;
import mobi.eyeline.informer.admin.WebConfig;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

import static org.junit.Assert.assertEquals;

/**
 * @author Aleksandr Khalitov
 */
public class WebConfigTest {

  private static WebConfig config;

  @BeforeClass
  public static void init() throws Exception{
    InputStream is = null;
    try{
      is = WebConfigTest.class.getResourceAsStream("webconfig.xml");
      config = new WebConfig(is);
    }finally {
      if(is != null) {
        try{
          is.close();
        }catch (IOException e){}
      }
    }
  }

  @Test(expected = IllegalArgumentException.class)
  public void testNull1() throws InitException {
    InputStream is = null;
    new WebConfig(is);
  }
  
  @Test(expected = IllegalArgumentException.class)
  public void testNull2() throws InitException {
    File f = null;
    new WebConfig(f);
  }

  @Test
  public void testData() throws Exception{    
    Assert.assertEquals(config.getInstallationType(), InstallationType.HA);
    assertEquals(config.getHSDaemonHost(), "localhost");
    assertEquals(config.getSingleDaemonHost(), "localhost");
    assertEquals(config.getHSDaemonPort(), 10000);
    assertEquals(config.getSingleDaemonPort(), 10000);
    assertEquals(config.getAppMirrorDirs().length, 1);
    assertEquals(config.getAppMirrorDirs()[0].getAbsoluteFile(), new File("mirror").getAbsoluteFile());
    assertEquals(config.getHSDaemonHosts().size(), 1);
    assertEquals(config.getHSDaemonHosts().iterator().next(),"sunfire");
    assertEquals(config.getJournalDir(), "journal");
  }

}
