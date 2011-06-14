package mobi.eyeline.informer.admin.monitoring;

import mobi.eyeline.informer.admin.AdminException;

import javax.management.Notification;
import javax.management.NotificationFilter;
import javax.management.NotificationListener;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Map;
import java.util.Properties;

/**
 * @author Aleksandr Khalitov
 */
public class InformerNotificationListener implements NotificationListener, NotificationFilter {

  private MonitoringJournal dataSource = new MonitringMemoryJournal();

  private static InformerNotificationListener instance = new InformerNotificationListener();


  public static synchronized void setJournal(MonitoringJournal dataSource) throws AdminException{
    if(dataSource == null) {
      throw new NullPointerException();
    }
    MonitoringJournal old = instance.dataSource;
    instance.dataSource = dataSource;
    if(old != null) {
      old.visit(new MonitoringEventsFilter(), new MonitoringJournal.Visitor() {
        @Override
        public boolean visit(MonitoringEvent e) throws AdminException {
          instance.dataSource.addEvents(e);
          return true;
        }
      });

    }
  }

  public static InformerNotificationListener getInstance() {
    return instance;
  }


  private static String notificationToString(Notification n) {
    StringBuilder sb = new StringBuilder();
    sb.append(n.getType()).append(" : ")
        .append(n.getMessage());
    Properties props = (Properties) n.getUserData();
    if (props != null) {
      sb.append(" (");
      int i = 0;
      for (Map.Entry e : props.entrySet()) {
        if (i > 0)
          sb.append(", ");
        sb.append(e.getKey()).append('=').append(e.getValue());
        i = 1;
      }
      sb.append(')');
    }
    return sb.toString();
  }

  private static void setEventProperties(MonitoringEvent t, Properties props) {
    for (Map.Entry e : props.entrySet())
      t.setProperty((String) e.getKey(), (String) e.getValue());
  }

  private static MonitoringEvent createInternalEvent(Notification n, MonitoringEvent.Severity s) {
    Properties props = (Properties) n.getUserData();
    String errorId = props.getProperty("errorId");

    String alarmId = "INTERNAL";
    if (errorId != null)
      alarmId += ':' + n.getUserData().toString();

    MonitoringEvent t = new MonitoringEvent(alarmId);
    t.setSeverity(s);
    return t;
  }

  private static MonitoringEvent createInteractionEvent(Notification n, MonitoringEvent.Severity s) {
    Properties props = (Properties) n.getUserData();
    String host = props.getProperty("host");
    String port = props.getProperty("port");

    if (host == null)
      host = "Unknown";
    if (port == null)
      port = "Unknown";

    MonitoringEvent t = new MonitoringEvent("INTERACTION:" + host + ':' + port);
    t.setSeverity(s);
    return t;
  }

  private static MonitoringEvent createConfigurationEvent(Notification n, MonitoringEvent.Severity s) {
    MonitoringEvent t = new MonitoringEvent("CONFIGURATION");
    t.setSeverity(s);
    return t;
  }

  private static MonitoringEvent createStatusChangeEvent(Notification n, MonitoringEvent.Severity s) {
    MonitoringEvent t = new MonitoringEvent("STATUS_CHANGE");
    t.setSeverity(s);
    return t;
  }

  private static MonitoringEvent createResourceLimitEvent(Notification n, MonitoringEvent.Severity s) {
    Properties props = (Properties) n.getUserData();
    String resourceId = props.getProperty("resourceId");

    MonitoringEvent t = new MonitoringEvent("RESOURCE_LIMIT:" + resourceId);
    t.setSeverity(s);
    return t;
  }

  public void handleNotification(Notification notification, Object handback) {
    String type = notification.getType();
    SimpleDateFormat sdf = new SimpleDateFormat("yyyy.MM.dd HH:mm:ss");
    System.out.println(sdf.format(new Date()) + " JMX: " +  notificationToString(notification));

    MonitoringEvent event = null;
    if (type.equals(ProgramMBean.NOTIFICATION_TYPE_STARTUP)) {
      event = createStatusChangeEvent(notification, MonitoringEvent.Severity.NORMAL);

    } else if (type.equals(ProgramMBean.NOTIFICATION_TYPE_SHUTDOWN)) {
      event = createStatusChangeEvent(notification, MonitoringEvent.Severity.CRITICAL);

    } else if (type.equals(ProgramMBean.NOTIFICATION_TYPE_CONFIGURATION_ERROR)) {
      event = createConfigurationEvent(notification, MonitoringEvent.Severity.CRITICAL);

    } else if (type.equals(ProgramMBean.NOTIFICATION_TYPE_CONFIGURATION_OK)) {
      event = createConfigurationEvent(notification, MonitoringEvent.Severity.NORMAL);

    } else if (type.equals(ProgramMBean.NOTIFICATION_TYPE_INTERACTION_ERROR)) {
      event = createInteractionEvent(notification, MonitoringEvent.Severity.CRITICAL);

    } else if (type.equals(ProgramMBean.NOTIFICATION_TYPE_INTERACTION_OK)) {
      event = createInteractionEvent(notification, MonitoringEvent.Severity.NORMAL);

    } else if (type.equals(ProgramMBean.NOTIFICATION_TYPE_INTERNAL_ERROR)) {
      event = createInternalEvent(notification, MonitoringEvent.Severity.MAJOR);

    } else if (type.equals(ProgramMBean.NOTIFICATION_TYPE_INTERNAL_OK)) {
      event = createInternalEvent(notification, MonitoringEvent.Severity.NORMAL);

    } else if (type.equals(ProgramMBean.NOTIFICATION_TYPE_RESOURCE_LIMIT_ERROR)) {
      event = createResourceLimitEvent(notification, MonitoringEvent.Severity.CRITICAL);
    }

    if (event != null) {
      event.setSource((MBean.Source)notification.getSource());
      event.setText(notification.getMessage());
      setEventProperties(event, (Properties) notification.getUserData());
      try {
        dataSource.addEvents(event);
      } catch (Exception ignored) {}
    }
  }

  public boolean isNotificationEnabled(Notification notification) {
    return notification.getType().startsWith(ProgramMBean.DOMAIN);
  }


}
