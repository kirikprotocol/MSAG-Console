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
    props.putAll(e.props);
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

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    MonitoringEvent event = (MonitoringEvent) o;

    if (time != event.time) return false;
    if (alarmId != null ? !alarmId.equals(event.alarmId) : event.alarmId != null) return false;
    if (severity != event.severity) return false;
    if (source != event.source) return false;
    if (text != null ? !text.equals(event.text) : event.text != null) return false;

    if(props.size() != event.props.size()) {
      return false;
    }

    for(Map.Entry<String, String> e : props.entrySet()) {
      if(!event.props.containsKey(e.getKey()) || !e.getValue().equals(event.props.get(e.getKey()))) {
        return false;
      }
    }
    return true;
  }

  @Override
  public int hashCode() {
    int result = text != null ? text.hashCode() : 0;
    result = 31 * result + (severity != null ? severity.hashCode() : 0);
    result = 31 * result + (alarmId != null ? alarmId.hashCode() : 0);
    result = 31 * result + (source != null ? source.hashCode() : 0);
    result = 31 * result + (int) (time ^ (time >>> 32));
    result = 31 * result + (props != null ? props.hashCode() : 0);
    return result;
  }

  public void setProperty(String name, String value) {
    props.put(name, value);
  }

  public static enum Severity {
    CRITICAL, MAJOR, MINOR, NORMAL
  }
}
