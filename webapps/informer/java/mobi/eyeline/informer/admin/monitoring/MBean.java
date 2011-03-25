package mobi.eyeline.informer.admin.monitoring;

import java.io.File;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
public class MBean {

  private static Map<Source, MBean> instances = new HashMap<Source, MBean>();
  private static ProgramMBean mbean = ProgramMBean.getInstance();

  public static enum Source {
    DCP, SIEBEL, SYSTEM, DELIVERY_CHANGELOG, CONTENT_PROVIDER          //todo
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
  private Map<String, Boolean> configurationState = Collections.synchronizedMap(new HashMap<String, Boolean>());
  private Map<String, Boolean> interactionState = Collections.synchronizedMap(new HashMap<String, Boolean>());

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
    Boolean state = configurationState.get(file.getAbsolutePath());
    if (state == null || state == Boolean.TRUE) {
      mbean.configurationError(source, file.getAbsolutePath(), message, atts);
      configurationState.put(file.getAbsolutePath(), Boolean.FALSE);
    }
  }

  public void notifyConfigurationOk(File file, String ... atts) {
    Boolean state = configurationState.get(file.getAbsolutePath());
    if (state == null || state == Boolean.FALSE) {
      mbean.configurationOk(source, file.getAbsolutePath(), "", atts);
      configurationState.put(file.getAbsolutePath(), Boolean.TRUE);
    }
  }

  public void notifyConfigurationOk(String ... atts) {
    mbean.configurationOk(source, atts);
  }

  public void notifyResourceLimitError(String resourceId, String message, String ... atts) {
    mbean.resourceLimitError(source, resourceId, message, atts);
  }

  public void notifyInteractionError(String externalComponent, String message, String ... atts) {
    Boolean state = interactionState.get(externalComponent);
    if (state == null || state == Boolean.TRUE) {
      mbean.interactionError(source, externalComponent, 0, message, atts);
      interactionState.put(externalComponent, Boolean.FALSE);
    }
  }

  public void notifyInteractionOk(String externalComponent, String ... atts) {
    Boolean state = interactionState.get(externalComponent);
    if (state == null || state == Boolean.FALSE) {
      mbean.interactionOk(source, externalComponent, 0, atts);
      interactionState.put(externalComponent, Boolean.TRUE);
    }
  }

  public void notifyInternalError(String errorId, String message, String ... atts) {
    mbean.internalError(source, errorId, message, atts);
  }

  public void notifyInternalOk(String errorId, String ... atts) {
    mbean.internalOk(source, errorId, atts);
  }

  public static void shutdown() {
    try{
      mbean.shutdown();
    }catch (Exception ignored){}
  }



}
