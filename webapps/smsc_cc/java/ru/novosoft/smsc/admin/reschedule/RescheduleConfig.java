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

  private String defaultReschedule;
  private int rescheduleLimit;
  private Collection<Reschedule> reschedules;

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

  public void load(InputStream is) throws Exception {
    XmlConfig config = new XmlConfig();
    config.load(is);
    load(config);
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

  public void save(InputStream is, OutputStream os) throws Exception {
    XmlConfig config = new XmlConfig();
    config.load(is);
    save(config);
    config.save(os);
  }

}
