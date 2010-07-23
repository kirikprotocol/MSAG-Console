package ru.novosoft.smsc.changelog.fraud;

import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.*;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.fraud.FraudManager;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.TestChangeLogListener;

/**
 * @author Artem Snopkov
 */
public class FraudManagerChangeLogAspectTest {

  FraudManager fm;
  TestChangeLogListener l;

  @Before
  public void setUp() throws Exception {
    AdminContext ctx = new TestAdminContext();
    ChangeLog changeLog = ChangeLog.getInstance(ctx);

    l = new TestChangeLogListener();
    changeLog.addListener(l);

    fm = ctx.getFraudManager();
  }

  @Test
  public void testLogSetter() throws Exception {
    int oldTail = fm.getTail();
    fm.setTail(oldTail + 1);


    assertEquals(1, l.changed_calls);
    assertEquals(ChangeLog.Subject.FRAUD, l.changed_subject);
    assertEquals("tail", l.changed_propertyName);
    assertEquals(oldTail, l.changed_oldValue);
    assertEquals(oldTail + 1, l.changed_newValue);
  }

  @Test
  public void testLogApply() throws Exception {
    fm.apply();

    assertEquals(1, l.apply_calls);
    assertEquals(ChangeLog.Subject.FRAUD, l.apply_subject);
  }

  @Test
  public void testLogReset() throws Exception {
    fm.reset();

    assertEquals(1, l.reset_calls);
    assertEquals(ChangeLog.Subject.FRAUD, l.reset_subject);
  }
}
