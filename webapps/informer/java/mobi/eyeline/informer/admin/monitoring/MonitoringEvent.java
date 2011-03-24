package mobi.eyeline.informer.admin.monitoring;

import java.util.HashMap;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
public class MonitoringEvent {

  private String text;

  private Severity severity;

  private String alarmId;

  private MBean.Source source;

  private long time = System.currentTimeMillis();

  private Map<String,String> props = new HashMap<String, String>(2);

  public MonitoringEvent(MonitoringEvent e) {
    alarmId = e.alarmId;
    severity = e.severity;
    text = e.text;
    source = e.source;
    time = e.time;
    for(Map.Entry<String, String> ent : props.entrySet()) {
      props.put(ent.getKey(), ent.getValue());
    }
  }

  public MonitoringEvent(String alarmId) {
    this.alarmId = alarmId;
  }

  public long getTime() {
    return time;
  }

  public void setTime(long time) {
    this.time = time;
  }

  public String getAlarmId() {
    return alarmId;
  }

  public String getText() {
    return text;
  }

  public void setText(String text) {
    this.text = text;
  }

  public Severity getSeverity() {
    return severity;
  }

  public void setSeverity(Severity severity) {
    this.severity = severity;
  }

  public MBean.Source getSource() {
    return source;
  }

  public void setSource(MBean.Source source) {
    this.source = source;
  }

  public Map<String, String> getProps() {
    return props;
  }

  public String getProperty(String name) {
    return props.get(name);
  }


  public void setProperty(String name, String value) {
    props.put(name, value);
  }

  public static enum Severity {
    CRITICAL, MAJOR, MINOR, NORMAL
  }
}
