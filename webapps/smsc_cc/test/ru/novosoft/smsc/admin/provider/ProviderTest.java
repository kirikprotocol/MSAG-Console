package ru.novosoft.smsc.admin.provider;

import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class ProviderTest {
  @Test(expected = AdminException.class)
  public void testSetName() throws Exception {
    new Provider(1, "");
  }

  @Test(expected = AdminException.class)
  public void testSetName1() throws Exception {
    new Provider(1, null);
  }

  @Test(expected = AdminException.class)
  public void testSetName2() throws Exception {
    new Provider(1, "ok").setName(null);
  }

  @Test(expected = AdminException.class)
  public void testSetName3() throws Exception {
    new Provider(1, "ok").setName("");
  }

  @Test
  public void testSetName4() throws Exception {
    new Provider(1, "ok");
  }
}
