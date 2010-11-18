package mobi.eyeline.informer.admin.users;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 14.10.2010
 * Time: 12:17:19
 */
public class UserTestUtils {

  static void compareUsers(User o, User n) {
    assertEquals(o.getLogin(),n.getLogin());
    assertEquals(o.getPhone(), n.getPhone());
    assertEquals(o.getOrganization(), n.getOrganization());
    assertEquals(o.getEmail(), n.getEmail());
    assertEquals(o.getFirstName(), n.getFirstName());
    assertEquals(o.getLastName(), n.getLastName());
    assertEquals(o.getPassword(), n.getPassword());
    assertEquals(o.getStatus(), n.getStatus());
    assertEquals(o.getLocale(), n.getLocale());
    assertEquals(o.isCreateCDR(), n.isCreateCDR());
    assertEquals(o.getSmsPerSec(), n.getSmsPerSec());
    assertEquals(o.getDeliveryStartTime(), n.getDeliveryStartTime());
    assertEquals(o.getDeliveryEndTime(), n.getDeliveryEndTime());
    assertEquals(o.getValidHours(), n.getValidHours());
    assertTrue(o.getRoles().equals(n.getRoles()));
    assertTrue(o.getDeliveryDays().equals(n.getDeliveryDays()));
    assertEquals(o.getDeliveryType(),n.getDeliveryType());
    assertEquals(o.isTransactionMode(), n.isTransactionMode());
    assertTrue(o.getRegions().equals(n.getRegions()));
    assertEquals(o.getPriority(),n.getPriority());
    assertEquals(o.isSmsNotification(),n.isSmsNotification());
    assertEquals(o.isEmailNotification(),n.isEmailNotification());
    assertEquals(o.isCreateArchive(),n.isCreateArchive());
    assertEquals(o.getDeliveryLifetime(),n.getDeliveryLifetime());

    assertEquals(o.getDirectory(),n.getDirectory());    
    assertEquals(o.isCreateReports(),n.isCreateReports());
    assertEquals(o.getReportsLifetime(),n.getReportsLifetime());
  }
}
