package mobi.eyeline.informer.admin.blacklist;

import com.eyelinecom.whoisd.personalization.PersonalizationClientPool;
import com.eyelinecom.whoisd.personalization.exceptions.PersonalizationClientException;
import mobi.eyeline.informer.admin.AdminException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
@SuppressWarnings({"EmptyCatchBlock"})
public class BlacklistManagerImplTest {

  private static BlackListManagerImpl manager;

  private static PersonalizationClientPool pool;

  @BeforeClass
  public static void before() throws IOException, PersonalizationClientException {
    Properties props = new Properties();
    InputStream is = null;
    try{
      is = BlacklistManagerImplTest.class.getResourceAsStream("personalization.properties");
      props.load(is);
    }finally {
      if(is != null) {
        try{
          is.close();
        }catch (IOException e){}
      }
    }
    manager = new BlackListManagerImpl(pool = new PersonalizationClientPool(props));
  }

  @Test
  public void addGetRemove() throws AdminException {
    
    manager.add("79139489906");
    assertTrue(manager.contains("+79139489906"));
    assertTrue(manager.contains("89139489906"));
    assertTrue(manager.contains("79139489906"));
    manager.remove("79139489906");
    assertFalse(manager.contains("79139489906"));
    assertFalse(manager.contains("89139489906"));
    assertFalse(manager.contains("+79139489906"));

    manager.add("+79139489906");
    assertTrue(manager.contains("+79139489906"));
    assertTrue(manager.contains("89139489906"));
    assertTrue(manager.contains("79139489906"));
    manager.remove("+79139489906");
    assertFalse(manager.contains("79139489906"));
    assertFalse(manager.contains("89139489906"));
    assertFalse(manager.contains("+79139489906"));

    manager.add("89139489906");
    assertTrue(manager.contains("+79139489906"));
    assertTrue(manager.contains("89139489906"));
    assertTrue(manager.contains("79139489906"));
    manager.remove("89139489906");
    assertFalse(manager.contains("79139489906"));
    assertFalse(manager.contains("89139489906"));
    assertFalse(manager.contains("+79139489906"));

    manager.add("19139489906");
    assertTrue(manager.contains("19139489906"));
    manager.remove("19139489906");
    assertFalse(manager.contains("19139489906"));
  }

  @Test
  public void addAllGetRemove() throws AdminException {
    List<String> list = new ArrayList<String>(2){{
      add("+79529223755"); add("+79139489906");
    }};
    manager.add(list);
    assertTrue(manager.contains("+79139489906"));
    assertTrue(manager.contains("+79529223755"));

    manager.remove(list);

    assertFalse(manager.contains("+79139489906"));
    assertFalse(manager.contains("+79529223755"));

  }

  @AfterClass
  public static void after() {
    if(pool != null) {
      try {
        pool.shutdown();
      } catch (PersonalizationClientException e) {}
    }

  }
}
