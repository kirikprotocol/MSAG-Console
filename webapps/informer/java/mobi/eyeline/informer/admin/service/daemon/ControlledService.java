package mobi.eyeline.informer.admin.service.daemon;

import java.io.File;

/**
 * Сервис, управляемый демоном
 *
 * @author Artem Snopkov
 */
public class ControlledService {

  private final String id; // Идентификатор
  private final boolean autostart; // Надо ли делать автостарт
  private final String args; // Список аргументов, передаваемых при старте
  private final String host; // Хост, на котором запущен сервис
  private final File baseDir; // Директория с сервисом

  ControlledService(String id, String host, boolean autostart, File baseDir, String args) {
    this.id = id;
    this.host = host;
    this.autostart = autostart;
    this.baseDir = baseDir;
    this.args = args;
  }

  public String getId() {
    return id;
  }

  public String getHost() {
    return host;
  }

  public File getBaseDir() {
    return baseDir;
  }

  public boolean isAutostart() {
    return autostart;
  }

  public String getArgs() {
    return args;
  }

}
