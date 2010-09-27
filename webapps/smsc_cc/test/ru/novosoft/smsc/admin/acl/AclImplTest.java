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
public class AclImplTest {

  @Test
  public void testCheckInfo() throws Exception {
    try {
      AclImpl.checkInfo(null, null);
      assertFalse(true);
    } catch (ValidationException ignored) {}

    try {
      AclImpl.checkInfo("", null);
      assertFalse(true);
    } catch (ValidationException ignored) {}

    try {
      StringBuilder name = new StringBuilder(AclImpl.MAX_NAME_LEN);
      for (int i=0; i<AclImpl.MAX_NAME_LEN + 1; i++)
        name.append("a");
      AclImpl.checkInfo(name.toString() , null);
      assertFalse(true);
    } catch (ValidationException ignored) {}
    
    try {
      StringBuilder desc = new StringBuilder(AclImpl.MAX_DESCRIPTION_LEN);
      for (int i=0; i<AclImpl.MAX_DESCRIPTION_LEN + 1; i++)
        desc.append("a");
      AclImpl.checkInfo("ok", desc.toString());
      assertFalse(true);
    } catch (ValidationException ignored) {}
  }

  @Test(expected = ValidationException.class)
  public void testCheckAddresses() throws Exception {
    ArrayList<Address> addresses = new ArrayList<Address>();
    addresses.add(null);
    AclImpl.checkAddresses(addresses);
  }
}
