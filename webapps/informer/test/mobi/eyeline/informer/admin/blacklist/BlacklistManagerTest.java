package mobi.eyeline.informer.admin.blacklist;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class BlacklistManagerTest {

  private static BlacklistManager manager = new BlacklistManagerStub();

  public static void init(BlacklistManager m) {
    manager = m;
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


}
