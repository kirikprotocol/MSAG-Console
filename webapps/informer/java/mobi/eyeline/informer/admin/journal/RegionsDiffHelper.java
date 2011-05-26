package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.util.Address;

import java.lang.reflect.Method;
import java.util.Collection;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
class RegionsDiffHelper extends DiffHelper {

  RegionsDiffHelper() {
    super(Subject.REGIONS);
  }

  void logChanges(Region oldRegion, Region newRegion, Journal journal, String userName) throws AdminException {
    List<Method> getters = getGetters(Region.class, "getMasks", "getTimeZone");
    List<Object> oldValues = callGetters(getters, oldRegion);
    List<Object> newValues = callGetters(getters, newRegion);
    logChanges(journal, oldValues, newValues, getters, userName, "region_property_changed", oldRegion.getName());

    Collection<Address> oldMasks = oldRegion.getMasks();
    Collection<Address> newMasks = newRegion.getMasks();

    for (Address o : oldMasks) {
      boolean removed = true;
      for (Address n : newMasks) {
        if (n.getSimpleAddress().equals(o.getSimpleAddress())) {
          removed = false;
          break;
        }
      }
      if (removed) {
        journal.addRecord(JournalRecord.Type.REMOVE, subject, userName, "region_mask_removed", o.getSimpleAddress(), newRegion.getName());
      }
    }

    for (Address n : newMasks) {
      boolean added = true;
      for (Address o : oldMasks) {
        if (o.getSimpleAddress().equals(n.getSimpleAddress())) {
          added = false;
          break;
        }
      }
      if (added) {
        journal.addRecord(JournalRecord.Type.ADD, subject, userName, "region_mask_added", n.getSimpleAddress(), newRegion.getName());
      }
    }

    if (!oldRegion.getTimeZone().getID().equals(newRegion.getTimeZone().getID())) {
      journal.addRecord(JournalRecord.Type.CHANGE, subject, userName, "region_property_changed", "timezone", oldRegion.getTimeZone().getDisplayName(), newRegion.getTimeZone().getDisplayName(), newRegion.getName());
    }
  }

  void logAddRegion(String newRegion, Journal journal, String userName) throws AdminException {
    journal.addRecord(JournalRecord.Type.ADD, subject, userName, "region_added", newRegion);
  }
  void logUpdateAll(Journal journal, String userName) throws AdminException {
    journal.addRecord(JournalRecord.Type.CHANGE, subject, userName, "config_update");
  }

  void logRemoveRegion(String regionName, Journal journal, String userName) throws AdminException {
    journal.addRecord(JournalRecord.Type.REMOVE, subject, userName, "region_removed", regionName);
  }

  void logSetDefault(Journal journal, int oldValue, int newValue, String userName) throws AdminException {
    journal.addRecord(JournalRecord.Type.CHANGE, subject, userName, "region_def_changed", Integer.toString(oldValue), Integer.toString(newValue));
  }
}
