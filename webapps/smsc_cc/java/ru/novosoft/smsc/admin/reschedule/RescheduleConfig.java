package ru.novosoft.smsc.admin.reschedule;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.util.ConfigHelper;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigException;
import ru.novosoft.smsc.util.config.XmlConfigParam;
import ru.novosoft.smsc.util.config.XmlConfigSection;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * @author Artem Snopkov
 */
class RescheduleConfig {

  private final File rescheduleFile;
  private final File backupDir;
  private final FileSystem fileSystem;

  private String defaultReschedule;
  private int rescheduleLimit;
  private Collection<Reschedule> reschedules;

  RescheduleConfig(File rescheduleFile, File backupDir, FileSystem fileSystem) {
    this.rescheduleFile = rescheduleFile;
    this.backupDir = backupDir;
    this.fileSystem = fileSystem;
  }

  public void setRescheduleLimit(int rescheduleLimit) {
    this.rescheduleLimit = rescheduleLimit;
  }

  public int getRescheduleLimit() {
    return rescheduleLimit;
  }

  public String getDefaultReschedule() {
    return defaultReschedule;
  }

  public void setDefaultReschedule(String intervals) {
    this.defaultReschedule = intervals;
  }

  public Collection<Reschedule> getReschedules() {
    return reschedules;
  }

  public void setReschedules(Collection<Reschedule> reschedules) {
    List<Reschedule> r = new ArrayList<Reschedule>(reschedules.size());
    for (Reschedule rr : reschedules)
      r.add(new Reschedule(rr));
    this.reschedules = r;
  }

  private XmlConfig loadConfig() throws XmlConfigException, AdminException {
    InputStream is = null;
    try {
      is = fileSystem.getInputStream(rescheduleFile);
      return new XmlConfig(is);
    } finally {
      if (is != null) {
        try {
          is.close();
        } catch (IOException e) {
        }
      }
    }
  }

  protected void load(XmlConfig config) throws XmlConfigException, AdminException {
    XmlConfigSection core = config.getSection("core");

    rescheduleLimit = core.getInt("reschedule_limit");
    defaultReschedule = core.getString("reschedule_table");

    if (reschedules == null)
      reschedules = new ArrayList<Reschedule>();
    reschedules.clear();
    for (XmlConfigParam rescheduleParam : core.getSection("reschedule table").params()) {
      Reschedule r = new Reschedule(rescheduleParam.getName(), rescheduleParam.getIntArray(","));
      reschedules.add(r);
    }
  }

  void load() throws AdminException{
    try {
      load(loadConfig());
    } catch (XmlConfigException e) {
      throw new RescheduleException("invalid_config_file_format", e);
    }
  }

  protected void save(XmlConfig config) throws XmlConfigException {
    XmlConfigSection core = config.getSection("core");

    core.setInt("reschedule_limit", rescheduleLimit);
    core.setString("reschedule_table", defaultReschedule == null ? "" : defaultReschedule);

    XmlConfigSection rescheduleTableSec = core.getSection("reschedule table");
    rescheduleTableSec.clear();

    if (reschedules != null) {
      for (Reschedule r : reschedules)
        rescheduleTableSec.setIntList(r.getIntervals(), r.getStatuses(), ",");
    }
  }

  void save() throws AdminException {
    try {
      XmlConfig config = loadConfig();

      save(config);

      ConfigHelper.saveXmlConfig(config, rescheduleFile, backupDir, fileSystem);

    } catch (XmlConfigException e) {
      throw new RescheduleException("config_file_save_error", e);
    }
  }

}
