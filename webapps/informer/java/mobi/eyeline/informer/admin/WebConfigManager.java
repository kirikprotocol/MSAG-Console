package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.cdr.CdrSettings;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.notifications.NotificationSettings;
import mobi.eyeline.informer.admin.siebel.SiebelSettings;
import mobi.eyeline.informer.admin.util.config.BaseManager;
import mobi.eyeline.informer.admin.util.config.SettingsReader;
import mobi.eyeline.informer.admin.util.config.SettingsWriter;

import java.io.File;
import java.util.Collection;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 11.11.2010
 * Time: 15:07:16
 */
class WebConfigManager extends BaseManager<WebConfigSettings> {


  public WebConfigManager(File config, File backup, FileSystem fileSys) throws InitException {
    super(null, config, backup, fileSys, new WebConfig());
    if (fileSys == null) {
      throw new InitException("FileSys is null");
    }
    if (backup == null) {
      throw new InitException("backup dir is null");
    }
  }

  /**
   * Возвращает тип установки Informer: SINGLE, HA или HS
   *
   * @return тип установки
   */
  public InstallationType getInstallationType() {
    return readSettings(new SettingsReader<WebConfigSettings, InstallationType>() {
      public InstallationType executeRead(WebConfigSettings settings) {
        return settings.getInstallationType();
      }
    });
  }

  /**
   * Возвращает хост для демона при Single-установке
   *
   * @return хост для демона при Single-установке
   */
  public String getSingleDaemonHost() {
    return readSettings(new SettingsReader<WebConfigSettings, String>() {
      public String executeRead(WebConfigSettings settings) {
        return settings.getSingleDaemonHost();
      }
    });
  }


  /**
   * Возвращает порт для демона при Single-установке
   *
   * @return порт для демона при Single-установке
   */
  public int getSingleDaemonPort() {
    return readSettings(new SettingsReader<WebConfigSettings, Integer>() {
      public Integer executeRead(WebConfigSettings settings) {
        return settings.getSingleDaemonPort();
      }
    });
  }


  /**
   * Возвращает директории-зеркала (HS-установка)
   *
   * @return директории-зеркала
   */
  public File[] getAppMirrorDirs() {
    return readSettings(new SettingsReader<WebConfigSettings, File[]>() {
      public File[] executeRead(WebConfigSettings settings) {
        return settings.getAppMirrorDirs();
      }
    });
  }

  /**
   * Возвращает хост для демона при HS-установке
   *
   * @return хост для демона при HS-установке
   */
  public String getHSDaemonHost() {
    return readSettings(new SettingsReader<WebConfigSettings, String>() {
      public String executeRead(WebConfigSettings settings) {
        return settings.getHSDaemonHost();
      }
    });
  }

  /**
   * Возвращает порт для демона при HS-установке
   *
   * @return порт для демона при HS-установке
   */
  public int getHSDaemonPort() {
    return readSettings(new SettingsReader<WebConfigSettings, Integer>() {
      public Integer executeRead(WebConfigSettings settings) {
        return settings.getHSDaemonPort();
      }
    });
  }

  /**
   * Возвращает список нод
   *
   * @return список нод
   */
  public Collection<String> getHSDaemonHosts() {
    return readSettings(new SettingsReader<WebConfigSettings, Collection<String>>() {
      public Collection<String> executeRead(WebConfigSettings settings) {
        return settings.getHSDaemonHosts();
      }
    });
  }


  /**
   * Возвращает директория журнала
   *
   * @return директория журнала
   */
  public String getJournalDir() {
    return readSettings(new SettingsReader<WebConfigSettings, String>() {
      public String executeRead(WebConfigSettings settings) {
        return settings.getJournalDir();
      }
    });
  }


  public String getWorkDir() {
    return readSettings(new SettingsReader<WebConfigSettings, String>() {
      public String executeRead(WebConfigSettings settings) {
        return settings.getWorkDir();
      }
    });
  }



  public NotificationSettings getNotificationSettings() {
    return readSettings(new SettingsReader<WebConfigSettings, NotificationSettings>() {
      public NotificationSettings executeRead(WebConfigSettings settings) {
        return settings.getNotificationSettings();
      }
    });

  }

  public void setNotificationSettings(final NotificationSettings props) throws AdminException {

    updateSettings(new SettingsWriter<WebConfigSettings>() {
      public void changeSettings(WebConfigSettings settings) throws AdminException {
        settings.setNotificationSettings(props);
      }

      public void infosmeCommand(Infosme infosme) throws AdminException {
        //dummy
      }
    });
  }

  public SiebelSettings getSiebelSettings() {
    return readSettings(new SettingsReader<WebConfigSettings, SiebelSettings>() {
      public SiebelSettings executeRead(WebConfigSettings settings) {
        return settings.getSiebelSettings();
      }
    });

  }

  public void setSiebelSettings(final SiebelSettings siebelSettings) throws AdminException {

    updateSettings(new SettingsWriter<WebConfigSettings>() {
      public void changeSettings(WebConfigSettings settings) throws AdminException {
        settings.setSiebelSettings(siebelSettings);
      }

      public void infosmeCommand(Infosme infosme) throws AdminException {
        //dummy
      }
    });
  }

  public CdrSettings getCdrSettings() {
    return readSettings(new SettingsReader<WebConfigSettings, CdrSettings>() {
      public CdrSettings executeRead(WebConfigSettings settings) {
        return settings.getCdrSettings();
      }
    });

  }

  public void setCdrSettings(final CdrSettings props) throws AdminException {

    updateSettings(new SettingsWriter<WebConfigSettings>() {
      public void changeSettings(WebConfigSettings settings) throws AdminException {
        settings.setCdrSettings(props);
      }

      public void infosmeCommand(Infosme infosme) throws AdminException {
        //dummy
      }
    });
  }

  public boolean isAllowUssdPushDeliveries() {
    return readSettings(new SettingsReader<WebConfigSettings,Boolean>(){
      public Boolean executeRead(WebConfigSettings settings) {
        return settings.isAllowUssdPushDeliveries();
      }
    });
  }

  public void setAllowUssdPushDeliveries(final boolean ussdPush) throws AdminException {
    updateSettings(new SettingsWriter<WebConfigSettings>() {
      public void changeSettings(WebConfigSettings settings) throws AdminException {
        settings.setAllowUssdPushDeliveries(ussdPush);
      }
      public void infosmeCommand(Infosme infosme) throws AdminException {
        //dummy
      }
    });
  }

}
