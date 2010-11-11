package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.util.config.BaseManager;
import mobi.eyeline.informer.admin.util.config.SettingsReader;
import mobi.eyeline.informer.admin.util.config.SettingsWriter;

import java.io.File;
import java.util.Collection;
import java.util.Properties;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 11.11.2010
 * Time: 15:07:16
 */
public class WebConfigManager extends BaseManager<WebConfigSettings> {


  public WebConfigManager(File config, File backup, FileSystem fileSys) throws InitException {
    super(null, config, backup, fileSys ,new WebConfig());
    if(fileSys==null) {
      throw new InitException("FileSys is null");
    }
    if(backup==null) {
      throw new InitException("backup dir is null");
    }
  }

  /**
   * Возвращает тип установки Informer: SINGLE, HA или HS
   * @return тип установки
   * @throws AdminException ошибка чтения конфига
   */
  public  InstallationType getInstallationType() {
    return readSettings(new SettingsReader<WebConfigSettings,InstallationType>(){
      public InstallationType executeRead(WebConfigSettings settings) {
        return settings.getInstallationType();
      }
    });
  }

  /**
   * Возвращает хост для демона при Single-установке
   * @return хост для демона при Single-установке
   * @throws AdminException ошибка чтения конфига
   */
  public String getSingleDaemonHost() throws AdminException {
    return readSettings(new SettingsReader<WebConfigSettings,String>(){
      public String executeRead(WebConfigSettings settings) {
        return settings.getSingleDaemonHost();
      }
    });
  }


  /**
   * Возвращает порт для демона при Single-установке
   * @return порт для демона при Single-установке
   * @throws AdminException ошибка чтения конфига
   */
  public int getSingleDaemonPort() throws AdminException {
    return readSettings(new SettingsReader<WebConfigSettings,Integer>(){
      public Integer executeRead(WebConfigSettings settings) {
        return settings.getSingleDaemonPort();
      }
    });
  }


  /**
   * Возвращает директории-зеркала (HS-установка)
   * @return директории-зеркала
   * @throws AdminException ошибка чтения конфига
   */
  public File[] getAppMirrorDirs() throws AdminException {
    return readSettings(new SettingsReader<WebConfigSettings,File[]>(){
      public File[] executeRead(WebConfigSettings settings) {
        return settings.getAppMirrorDirs();
      }
    });
  }

  /**
   * Возвращает хост для демона при HS-установке
   * @return хост для демона при HS-установке
   * @throws AdminException ошибка чтения конфига
   */
  public String getHSDaemonHost() throws AdminException {
    return readSettings(new SettingsReader<WebConfigSettings,String>(){
      public String executeRead(WebConfigSettings settings) {
        return settings.getHSDaemonHost();
      }
    });
  }

  /**
   * Возвращает порт для демона при HS-установке
   * @return порт для демона при HS-установке
   * @throws AdminException ошибка чтения конфига
   */
  public int getHSDaemonPort() throws AdminException {
    return readSettings(new SettingsReader<WebConfigSettings,Integer>(){
      public Integer executeRead(WebConfigSettings settings) {
        return settings.getHSDaemonPort();
      }
    });
  }

  /**
   * Возвращает список нод
   * @return список нод
   * @throws AdminException ошибка чтения конфига
   */
  public Collection<String> getHSDaemonHosts() throws AdminException {
    return readSettings(new SettingsReader<WebConfigSettings,Collection<String>>(){
      public Collection<String> executeRead(WebConfigSettings settings) {
        return settings.getHSDaemonHosts();
      }
    });
  }


   /**
   * Возвращает директория журнала
   * @return директория журнала
   * @throws AdminException ошибка чтения конфига
   */
  public String getJournalDir() throws AdminException {
    return readSettings(new SettingsReader<WebConfigSettings,String>(){
      public String executeRead(WebConfigSettings settings) {
        return settings.getJournalDir();
      }
    });
  }

  public Properties getJavaMailProperties() {
    return readSettings(new SettingsReader<WebConfigSettings,Properties>(){
      public Properties executeRead(WebConfigSettings settings) {
        return settings.getJavaMailProperties();
      }
    });

  }

  public void setJavaMailProperties(final Properties props) throws AdminException {
    updateSettings(new SettingsWriter<WebConfigSettings>() {

      public void changeSettings(WebConfigSettings settings) throws AdminException {
        Properties copy = new Properties();
        for(String s : props.stringPropertyNames()) {
          copy.put(s,props.get(s));
        }
        settings.setJavaMailProperties(copy);
      }

      public void infosmeCommand(Infosme infosme) throws AdminException {
        //dummy
      }
    });
  }
}
