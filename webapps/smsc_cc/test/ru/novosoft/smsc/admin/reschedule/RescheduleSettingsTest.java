package ru.novosoft.smsc.admin.reschedule;

import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;

/**
 * @author Artem Snopkov
 */
public class RescheduleSettingsTest {

  @Test
  public void setSingleRescheduleTest() throws AdminException {
    Reschedule r1 = new Reschedule("1h", 12, 20, 30);

    Collection<Reschedule> res = new ArrayList<Reschedule>();
    Collections.addAll(res, r1);

    new RescheduleSettings().setReschedules(res);
  }

  @Test(expected = AdminException.class)
  public void setIntersectsReschedulesTest() throws AdminException {
    Reschedule r1 = new Reschedule("1h", 12, 20, 30);
    Reschedule r2 = new Reschedule("1h", 10, 20, 50);

    Collection<Reschedule> res = new ArrayList<Reschedule>();
    Collections.addAll(res, r1, r2);

    new RescheduleSettings().setReschedules(res);
  }

  @Test
  public void setNotIntersectsReschedulesTest() throws AdminException {
    Reschedule r1 = new Reschedule("1h", 12, 20, 30);
    Reschedule r2 = new Reschedule("1h", 10, 22, 50);

    Collection<Reschedule> res = new ArrayList<Reschedule>();
    Collections.addAll(res, r1, r2);

    new RescheduleSettings().setReschedules(res);
  }
}
