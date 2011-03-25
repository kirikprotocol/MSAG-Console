package mobi.eyeline.informer.admin.monitoring;

import org.junit.Test;

import static org.junit.Assert.assertEquals;

/**
 * @author Aleksandr Khalitov
 */
public class MonitoringEventTest {


  private static MonitoringEvent createEvent() {
    MonitoringEvent event = new MonitoringEvent("alm1");
    event.setProperty("prop1", "value1");
    event.setProperty("prop2", "value2");
    event.setSeverity(MonitoringEvent.Severity.NORMAL);
    event.setSource(MBean.Source.CONTENT_PROVIDER);
    event.setText("Dsadasdada!");
    event.setTime(12121212121l);
    return event;
  }

  @Test
  public void cloneTest() {
    MonitoringEvent e = createEvent();
    assertEquals(e, new MonitoringEvent(e));
  }
}
