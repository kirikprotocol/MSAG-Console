package mobi.eyeline.informer;

import com.eyelinecom.whoisd.personalization.PersonalizationClientPool;
import mobi.eyeline.informer.admin.blacklist.BlacklistManagerImpl;
import mobi.eyeline.informer.admin.blacklist.BlacklistManagerTest;
import org.junit.BeforeClass;
import org.junit.runner.RunWith;
import org.junit.runners.Suite;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

/**
 * @author Artem Snopkov
 */

@RunWith(Suite.class)
@Suite.SuiteClasses({BlacklistManagerTest.class})
public class IntegrationTests {

  private static PersonalizationClientPool pool;

  private static void initPvssPool() throws Exception {

    Properties props = new Properties();
    InputStream is = null;
    try{
      is = new FileInputStream("test.properties");
      props.load(is);
      pool = new PersonalizationClientPool(props);
    } catch (IOException e) {
      e.printStackTrace();
    } finally {
      if(is != null) {
        try{
          is.close();
        }catch (IOException e){}
      }
    }
  }

  @BeforeClass
  public static void setUp() throws Exception {
    initPvssPool();
    BlacklistManagerTest.init(new BlacklistManagerImpl(pool));
  }

}
