package ru.novosoft.smsc.admin.profile;

import org.junit.Test;
import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Address;

/**
 * @author Artem Snopkov
 */
public class ProfilesSetImplTest {

  @Test
  public void readTest() throws AdminException {
    ProfilesSetImpl ps = null;
    try {
      ps = new ProfilesSetImpl(ProfilesSetImplTest.class.getResourceAsStream("profiles.bin"), false);
      assertTrue(ps.next());
      assertEquals(new Address(".0.0.0"), ps.get().getAddress());
      assertTrue(ps.next());
      assertEquals(new Address(".0.1.79139495114"), ps.get().getAddress());
      assertTrue(ps.next());
      assertEquals(new Address(".0.0.321"), ps.get().getAddress());
      assertTrue(ps.next());
      assertEquals(new Address(".0.0.79139495115"), ps.get().getAddress());      
    } finally {
      if (ps != null)
        ps.close();
    }
  }
}
