package ru.novosoft.smsc.admin.reschedule;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigException;
import ru.novosoft.smsc.util.config.XmlConfigParam;
import ru.novosoft.smsc.util.config.XmlConfigSection;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * @author Artem Snopkov
 */
class RescheduleConfig implements ManagedConfigFile {

  private RescheduleSettings settings;

  public RescheduleSettings getSettings() {
    return settings;
  }

  public void setSettings(RescheduleSettings settings) {
    this.settings = settings;
  }

  protected void load(XmlConfig config) throws XmlConfigException, AdminException {
    RescheduleSettings s = new RescheduleSettings();
    XmlConfigSection core = config.getSection("core");

    s.setRescheduleLimit(core.getInt("reschedule_limit"));
    s.setDefaultReschedule(core.getString("reschedule_table"));

    List<Reschedule> reschedules = new ArrayList<Reschedule>();
    for (XmlConfigParam rescheduleParam : core.getSection("reschedule table").params()) {
      Reschedule r = new Reschedule(rescheduleParam.getName(), rescheduleParam.getIntArray(","));
      reschedules.add(r);
    }

    s.setReschedules(reschedules);

    settings = s;
  }

  public void load(InputStream is) throws Exception {
    XmlConfig config = new XmlConfig();
    config.load(is);
    load(config);
  }

  protected void save(XmlConfig config) throws XmlConfigException {
    XmlConfigSection core = config.getSection("core");

    core.setInt("reschedule_limit", settings.getRescheduleLimit());
    core.setString("reschedule_table", settings.getDefaultReschedule() == null ? "" : settings.getDefaultReschedule());

    XmlConfigSection rescheduleTableSec = core.getSection("reschedule table");
    rescheduleTableSec.clear();


    for (Reschedule r : settings.getReschedules())
      rescheduleTableSec.setIntList(r.getIntervals(), r.getStatuses(), ",");
  }

  public void save(InputStream is, OutputStream os) throws Exception {
    XmlConfig config = new XmlConfig();
    config.load(is);
    save(config);
    config.save(os);
  }

}
