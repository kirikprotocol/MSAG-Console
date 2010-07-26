package ru.novosoft.smsc.changelog.snmp;

import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;
import static org.junit.Assert.assertEquals;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.snmp.SnmpCounter;
import ru.novosoft.smsc.admin.snmp.SnmpManager;
import ru.novosoft.smsc.admin.snmp.SnmpObject;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.TestChangeLogListener;

import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class SnmpManagerChangeLogAspectTest {

  SnmpManager m;
  TestChangeLogListener l;

  @Before
  public void setUp() throws Exception {
    AdminContext ctx = new TestAdminContext();
    ChangeLog changeLog = ChangeLog.getInstance(ctx);
    l = new TestChangeLogListener();
    changeLog.addListener(l);

    m = ctx.getSnmpManager();
  }

  @Test
  public void testLogSetCounterInterval() throws Exception {
    int oldValue = m.getCounterInterval();
    m.setCounterInterval(oldValue + 1);

    assertEquals(1, l.changed_calls);
    assertEquals(ChangeLog.Subject.SNMP, l.changed_subject);
    assertEquals("counterInterval", l.changed_propertyName);
    assertEquals(oldValue, l.changed_oldValue);
    assertEquals(oldValue+1, l.changed_newValue);
  }

  @Test
  public void testLogSetDefaultSnmpObject() throws Exception {

    SnmpObject oldValue = m.getDefaultSnmpObject();

    SnmpObject newValue = new SnmpObject(oldValue); 
    newValue.setCounterDelivered(1,2,3,4);
    m.setDefaultSnmpObject(newValue);

    assertEquals(1, l.changed_calls);
    assertEquals(ChangeLog.Subject.SNMP, l.changed_subject);
    assertEquals("defaultSnmpObject", l.changed_subjectDesc);
    assertEquals("counterDelivered", l.changed_propertyName);
    assertEquals(oldValue.getCounterDelivered(), l.changed_oldValue);
    assertEquals(newValue.getCounterDelivered(), l.changed_newValue);
  }

  @Test
  public void testLogSetSnmpObjects() throws Exception {

    Map<String, SnmpObject> snmpObjects = m.getSnmpObjects();

    SnmpObject MY = snmpObjects.get("MY");
    MY.setCounterDelivered(1,2,3,4);

    snmpObjects.remove("GLOBAL");

    SnmpObject newMy = new SnmpObject(MY);
    snmpObjects.put("newMy", newMy);

    m.setSnmpObjects(snmpObjects);

    assertEquals(1, l.changed_calls);
    assertEquals(ChangeLog.Subject.SNMP, l.changed_subject);
    assertEquals("MY", l.changed_subjectDesc);
    assertEquals("counterDelivered", l.changed_propertyName);
    assertEquals(new SnmpCounter(0,0,0,0), l.changed_oldValue);
    assertEquals(MY.getCounterDelivered(), l.changed_newValue);

    assertEquals(1, l.removed_calls);
    assertEquals(ChangeLog.Subject.SNMP, l.removed_subject);
    assertEquals("GLOBAL", l.removed_object);

    assertEquals(1, l.added_calls);
    assertEquals(ChangeLog.Subject.SNMP, l.added_subject);
    assertEquals("newMy", l.added_object);
  }
}
