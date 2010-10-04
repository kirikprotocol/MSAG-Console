package mobi.eyeline.informer.admin.smsc;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
class SmscSettings {

  private Map<String, Smsc> smscs = new LinkedHashMap<String, Smsc>();

  private ValidationHelper vh = new ValidationHelper(SmscSettings.class);

  private String defaultSmsc;

  SmscSettings() {
  }

  List<Smsc> getSmscs() {
    return new ArrayList<Smsc>(smscs.values());
  }

  Smsc getSmsc(String name) {
    return smscs.get(name);
  }

  String getDefaultSmsc() {
    return defaultSmsc;
  }

  void setDefaultSmsc(String defaultSmsc) throws AdminException {
    vh.checkСontainsKey("default", smscs, defaultSmsc);
    this.defaultSmsc = defaultSmsc;
  }

  void addSmsc(Smsc smsc) throws AdminException {
    vh.checkNotСontainsKey("smsc_name", smscs, smsc.getName());
    smscs.put(smsc.getName(), smsc);
  }

  void updateSmsc(Smsc smsc) throws AdminException {
    vh.checkСontainsKey("smsc_name", smscs, smsc.getName());
    smscs.put(smsc.getName(), smsc);
  }

  void removeSmsc(String smsc) throws AdminException {
    smscs.remove(smsc);
  }
}
