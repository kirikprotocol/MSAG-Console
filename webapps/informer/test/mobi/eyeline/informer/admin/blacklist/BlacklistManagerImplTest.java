package mobi.eyeline.informer.admin.blacklist;

import com.eyelinecom.whoisd.personalization.PersonalizationClientPool;
import com.eyelinecom.whoisd.personalization.exceptions.PersonalizationClientException;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
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
    
    manager.add(new Address("79139489906"));
    assertTrue(manager.contains(new Address("+79139489906")));
    assertTrue(manager.contains(new Address("89139489906")));
    assertTrue(manager.contains(new Address("79139489906")));
    manager.remove(new Address("79139489906"));
    assertFalse(manager.contains(new Address("79139489906")));
    assertFalse(manager.contains(new Address("89139489906")));
    assertFalse(manager.contains(new Address("+79139489906")));

    manager.add(new Address("+79139489906"));
    assertTrue(manager.contains(new Address("+79139489906")));
    assertTrue(manager.contains(new Address("89139489906")));
    assertTrue(manager.contains(new Address("79139489906")));
    manager.remove(new Address("+79139489906"));
    assertFalse(manager.contains(new Address("79139489906")));
    assertFalse(manager.contains(new Address("89139489906")));
    assertFalse(manager.contains(new Address("+79139489906")));

    manager.add(new Address("89139489906"));
    assertTrue(manager.contains(new Address("+79139489906")));
    assertTrue(manager.contains(new Address("89139489906")));
    assertTrue(manager.contains(new Address("79139489906")));
    manager.remove(new Address("89139489906"));
    assertFalse(manager.contains(new Address("79139489906")));
    assertFalse(manager.contains(new Address("89139489906")));
    assertFalse(manager.contains(new Address("+79139489906")));

    manager.add(new Address("19139489906"));
    assertTrue(manager.contains(new Address("19139489906")));
    manager.remove(new Address("19139489906"));
    assertFalse(manager.contains(new Address("19139489906")));
  }

  @Test
  public void addAllGetRemove() throws AdminException {
    List<Address> list = new ArrayList<Address>(2){{
      add(new Address("+79529223755")); add(new Address("+79139489906"));
    }};
    manager.add(list);
    assertTrue(manager.contains(new Address("+79139489906")));
    assertTrue(manager.contains(new Address("+79529223755")));

    manager.remove(list);

    assertFalse(manager.contains(new Address("+79139489906")));
    assertFalse(manager.contains(new Address("+79529223755")));

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
