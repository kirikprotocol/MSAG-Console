package ru.novosoft.smsc.admin.acl;

import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationException;
import ru.novosoft.smsc.util.Address;

import java.util.ArrayList;

import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class AclTest {

  @Test
  public void testCheckInfo() throws Exception {
    try {
      Acl.checkInfo(null, null);
      assertFalse(true);
    } catch (ValidationException ignored) {}

    try {
      Acl.checkInfo("", null);
      assertFalse(true);
    } catch (ValidationException ignored) {}

    try {
      StringBuilder name = new StringBuilder(Acl.MAX_NAME_LEN);
      for (int i=0; i<Acl.MAX_NAME_LEN + 1; i++)
        name.append("a");
      Acl.checkInfo(name.toString() , null);
      assertFalse(true);
    } catch (ValidationException ignored) {}
    
    try {
      StringBuilder desc = new StringBuilder(Acl.MAX_DESCRIPTION_LEN);
      for (int i=0; i<Acl.MAX_DESCRIPTION_LEN + 1; i++)
        desc.append("a");
      Acl.checkInfo("ok", desc.toString());
      assertFalse(true);
    } catch (ValidationException ignored) {}
  }

  @Test(expected = ValidationException.class)
  public void testCheckAddresses() throws Exception {
    ArrayList<Address> addresses = new ArrayList<Address>();
    addresses.add(null);
    Acl.checkAddresses(addresses);
  }
}
