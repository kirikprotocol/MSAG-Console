package mobi.eyeline.informer.admin.monitoring;

import java.io.File;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class MBean {

  private static Map<Source, MBean> instances = new HashMap<Source, MBean>();
  private static ProgramMBean mbean = ProgramMBean.getInstance();

  public static enum Source {
    DCP, SIEBEL, SYSTEM          //todo
  }

  public synchronized static MBean getInstance(Source name) {
    MBean instance = instances.get(name);
    if (instance == null) {
      instance = new MBean(name);
      instances.put(name, instance);
    }
    return instance;
  }
  private Source source;
  private Set<String> openedConfigurationErrors = Collections.synchronizedSet(new HashSet<String>());
  private Set<String> openedInterationErrors = Collections.synchronizedSet(new HashSet<String>());

  private MBean(Source source) {
    this.source = source;
  }

  public void addProperty(String name, String description, PropertyWrapper wrapper) {
    mbean.addProperty(source, name, description, wrapper);
  }

  public void notifyStartup(String version, String ... atts) {
    mbean.startup(source, version, atts);
  }

  public void notifyShutdown(String ... atts) {
    mbean.shutdown(source, atts);
  }

  public void notifyConfigurationError(File file, String message, String ... atts) {
    openedConfigurationErrors.add(file.getAbsolutePath());
    mbean.configurationError(source, file.getAbsolutePath(), message, atts);
  }

  public void notifyConfigurationOk(File file, String ... atts) {
    if (openedConfigurationErrors.contains(file.getAbsolutePath())) {
      openedConfigurationErrors.remove(file.getAbsolutePath());
      mbean.configurationOk(source, file.getAbsolutePath(), "", atts);
    }
  }

  public void notifyConfigurationOk(String ... atts) {
    mbean.configurationOk(source, atts);
  }

  public void notifyResourceLimitError(String resourceId, String message, String ... atts) {
    mbean.resourceLimitError(source, resourceId, message, atts);
  }

  public void notifyInteractionError(String externalComponent, String message, String ... atts) {
    openedInterationErrors.add(externalComponent);
    mbean.interactionError(source, externalComponent, 0, message, atts);
  }

  public void notifyInteractionOk(String externalComponent, String ... atts) {
    if (openedInterationErrors.contains(externalComponent)) {
      openedInterationErrors.remove(externalComponent);
      mbean.interactionOk(source, externalComponent, 0, atts);
    }
  }

  public void notifyInternalError(String errorId, String message, String ... atts) {
    mbean.internalError(source, errorId, message, atts);
  }

  public void notifyInternalOk(String errorId, String ... atts) {
    mbean.internalOk(source, errorId, atts);
  }

}
