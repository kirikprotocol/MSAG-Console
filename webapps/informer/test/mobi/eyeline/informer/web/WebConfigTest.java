package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.InstallationType;
import mobi.eyeline.informer.admin.WebConfigManager;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.notifications.NotificationSettings;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;

import static org.junit.Assert.assertEquals;

/**
 * @author Aleksandr Khalitov
 */
public class WebConfigTest {

  private static File configFile,backupDir ;
  private static FileSystem fileSys;

  @BeforeClass
  public static void init() throws Exception{
    configFile = TestUtils.exportResourceToRandomFile(WebConfigTest.class.getResourceAsStream("webconfig.xml"), ".webconfig");
    backupDir = TestUtils.createRandomDir(".webconfig.backup");
    fileSys = FileSystem.getFSForSingleInst();
  }

  @AfterClass
  public static void shutdown() {
    if(configFile!= null) {
      configFile.delete();
    }
    if(backupDir != null) {
      TestUtils.recursiveDeleteFolder(backupDir);
    }
  }


  @Test(expected = InitException.class)
  public void testNull() throws InitException, AdminException {  
    new WebConfigManager(null,backupDir,fileSys);
  }

  @Test
  public void testData() throws Exception{
    WebConfigManager config = new WebConfigManager(configFile,backupDir,fileSys);

    //change settings to cause save of file
    NotificationSettings props = config.getNotificationSettings();
    props.setMailHost("kikimora");
    config.setNotificationSettings(props);

    props = config.getNotificationSettings();
    props.setSmsTemplateActivated("Wow! &#<> Delivery \"{0}\" state changed to '{1}' at {2}");
    config.setNotificationSettings(props);

    //check after rereading
    config = new WebConfigManager(configFile,backupDir,fileSys);
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

    props = config.getNotificationSettings();
    assertEquals("kikimora",props.getMailHost());
    assertEquals("arrivedok",props.getMailUser());
    assertEquals("12345qwe",props.getMailPassword());
    assertEquals("smtp",props.getMailProperties().get("mail.transport.protocol"));
    assertEquals("admin@informer.com",props.getMailFrom());


    assertEquals("Wow! &#<> Delivery \"{0}\" state changed to '{1}' at {2}",props.getSmsTemplateActivated());
    assertEquals("Delivery {0} finished  {1} ",props.getSmsTemplateFinished());
    assertEquals("Delivery state changed",props.getEmailSubjectTemplate());
    assertEquals("Delivery {0} activated at {1}. ",props.getEmailTemplateActivated());
    assertEquals("Delivery {0} finished  at {1}. ",props.getEmailTemplateFinished());

        

  }

}
