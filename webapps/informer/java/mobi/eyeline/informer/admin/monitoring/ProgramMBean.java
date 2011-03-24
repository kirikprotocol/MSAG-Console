package mobi.eyeline.informer.admin.monitoring;

import mobi.eyeline.apm.utils.jmx.mbeans.AbstractDynamicMBean;

import javax.management.*;
import java.lang.management.ManagementFactory;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * @author Aleksandr Khalitov
 */
class ProgramMBean extends AbstractDynamicMBean implements NotificationBroadcaster {

  static String DOMAIN = "mobi.eyeline.informer";

  static String NOTIFICATION_TYPE_STARTUP = DOMAIN + ".startup";
  static String NOTIFICATION_TYPE_SHUTDOWN = DOMAIN + ".shutdown";
  static String NOTIFICATION_TYPE_CONFIGURATION_ERROR = DOMAIN + ".configuration.error";
  static String NOTIFICATION_TYPE_CONFIGURATION_OK = DOMAIN + ".configuration.ok";
  static String NOTIFICATION_TYPE_INTERACTION_ERROR = DOMAIN + ".interaction.error";
  static String NOTIFICATION_TYPE_INTERACTION_OK = DOMAIN + ".interaction.ok";
  static String NOTIFICATION_TYPE_INTERNAL_ERROR = DOMAIN + ".internal.error";
  static String NOTIFICATION_TYPE_INTERNAL_OK = DOMAIN + ".internal.ok";
  static String NOTIFICATION_TYPE_RESOURCE_LIMIT_ERROR = DOMAIN + ".resource.limit.error";

  private static final ProgramMBean instance = new ProgramMBean();

  public static ProgramMBean getInstance() {
    return instance;
  }

  private final AtomicInteger seqNum = new AtomicInteger(0);

  private Map<String, PropertyWrapper> properties = new HashMap<String, PropertyWrapper>();

  private final NotificationBroadcasterSupport emmitter = new NotificationBroadcasterSupport();

  private ProgramMBean() {
    super(ProgramMBean.class, "Informer web MBean");

    MBeanServer mbs = ManagementFactory.getPlatformMBeanServer();
    try {
      ObjectName oname = new ObjectName(DOMAIN, "name", "Informer");
      mbs.registerMBean(this, oname);
      mbs.addNotificationListener(oname, InformerNotificationListener.getInstance(), InformerNotificationListener.getInstance(), null);

    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  public void addProperty(MBean.Source sourceId, String name, String description, PropertyWrapper wrapper) {
    String fullName = sourceId.toString() + '-' + name;
    attributes.add(new MBeanAttributeInfo(fullName, wrapper.getType(), description, wrapper.isReadable(), wrapper.isWritable(), wrapper.isIs()));
    properties.put(fullName, wrapper);
  }

  private static Properties createProps(Object source, String ... atts) {
    Properties props = new Properties();
    props.setProperty("src", source == null ? "null" : source.toString());
    if (atts != null && atts.length >= 2) {
      props = new Properties();
      for (int i = 0; i + 1 < atts.length; i += 2)
        props.setProperty(atts[i], atts[i + 1]);
    }
    return props;
  }

  public void startup(MBean.Source source, String version, String... atts) {
    Notification n = new Notification(NOTIFICATION_TYPE_STARTUP, this, seqNum.incrementAndGet(), "Startup");
    Properties props = createProps(source, atts);
    props.setProperty("version", version);
    n.setUserData(props);
    n.setSource(source);
    emmitter.sendNotification(n);
  }


  public void shutdown(MBean.Source source, String... atts) {
    Notification n = new Notification(NOTIFICATION_TYPE_SHUTDOWN, this, seqNum.incrementAndGet(), "Shutdown");
    n.setUserData(createProps(source, atts));
    n.setSource(source);
    emmitter.sendNotification(n);
  }


  public void configurationError(MBean.Source source, String file, String message, String... atts) {
    Notification n = new Notification(NOTIFICATION_TYPE_CONFIGURATION_ERROR, this, seqNum.incrementAndGet(), message);
    Properties props = createProps(source, atts);
    props.setProperty("file", file);
    n.setUserData(props);
    n.setSource(source);
    emmitter.sendNotification(n);
  }


  public void configurationOk(MBean.Source source, String file, String message, String... atts) {
    Notification n = new Notification(NOTIFICATION_TYPE_CONFIGURATION_OK, this, seqNum.incrementAndGet(), message);
    Properties props = createProps(source, atts);
    props.setProperty("file", file);
    n.setUserData(props);
    n.setSource(source);
    emmitter.sendNotification(n);
  }


  public void configurationOk(MBean.Source source, String... atts) {
    Notification n = new Notification(NOTIFICATION_TYPE_CONFIGURATION_OK, this, seqNum.incrementAndGet(), "Success");
    n.setUserData(createProps(source, atts));
    n.setSource(source);
    emmitter.sendNotification(n);
  }


  public void resourceLimitError(MBean.Source source, String resource, String message, String... atts) {
    Notification n = new Notification(NOTIFICATION_TYPE_RESOURCE_LIMIT_ERROR, this, seqNum.incrementAndGet(), message);
    Properties props = createProps(source, atts);
    props.setProperty("resourceId", resource);
    n.setUserData(props);
    n.setSource(source);
    emmitter.sendNotification(n);
  }


  public void internalOk(MBean.Source source, String errorId, String... atts) {
    Notification n = new Notification(NOTIFICATION_TYPE_INTERNAL_OK, this, seqNum.incrementAndGet());
    Properties props = createProps(source, atts);
    if (errorId != null)
      props.setProperty("errorId", errorId);
    n.setUserData(props);
    n.setSource(source);
    emmitter.sendNotification(n);
  }


  public void internalError(MBean.Source source, String errorId, String message, String... atts) {
    Notification n = new Notification(NOTIFICATION_TYPE_INTERNAL_ERROR, this, seqNum.incrementAndGet(), message);
    Properties props = createProps(source, atts);
    if (errorId != null)
      props.setProperty("errorId", errorId);
    n.setUserData(props);
    n.setSource(source);
    emmitter.sendNotification(n);
  }


  public void interactionError(MBean.Source source, String host, int port, String message, String... atts) {
    Notification n = new Notification(NOTIFICATION_TYPE_INTERACTION_ERROR, this, seqNum.incrementAndGet(), message);
    Properties props = createProps(source, atts);
    props.setProperty("host", host);
    props.setProperty("port", String.valueOf(port));
    n.setUserData(props);
    n.setSource(source);
    emmitter.sendNotification(n);
  }

  public void interactionOk(MBean.Source source, String host, int port, String... atts) {
    Notification n = new Notification(NOTIFICATION_TYPE_INTERACTION_OK, this, seqNum.incrementAndGet());
    Properties props = createProps(source, atts);
    props.setProperty("host", host);
    props.setProperty("port", String.valueOf(port));
    n.setUserData(props);
    n.setSource(source);
    emmitter.sendNotification(n);
  }

  public Object getAttribute(String attribute) throws AttributeNotFoundException, MBeanException, ReflectionException {
    PropertyWrapper w = properties.get(attribute);
    if (w == null)
      throw new AttributeNotFoundException(attribute);
    return w.getValue();
  }

  public void setAttribute(Attribute attribute) throws AttributeNotFoundException, InvalidAttributeValueException, MBeanException, ReflectionException {
    PropertyWrapper w = properties.get(attribute.getName());
    if (w == null)
      throw new AttributeNotFoundException(attribute.getName());
    w.setValue(attribute.getValue());
  }

  public Object invoke(String actionName, Object[] params, String[] signature) throws MBeanException, ReflectionException {
    return null;
  }

  public void addNotificationListener(NotificationListener listener, NotificationFilter filter, Object handback) throws IllegalArgumentException {
    emmitter.addNotificationListener(listener, filter, handback);
  }

  public void removeNotificationListener(NotificationListener listener) throws ListenerNotFoundException {
    emmitter.removeNotificationListener(listener);
  }

  public MBeanNotificationInfo[] getNotificationInfo() {
    return emmitter.getNotificationInfo();
  }
}