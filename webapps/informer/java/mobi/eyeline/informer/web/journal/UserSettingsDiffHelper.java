package mobi.eyeline.informer.web.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.users.User;
import mobi.eyeline.informer.web.users.UsersSettings;

import java.lang.reflect.Method;
import java.util.List;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
class UserSettingsDiffHelper extends DiffHelper {

  public UserSettingsDiffHelper(String subject) {
    super(subject);
  }

  public void logChanges(Journal j, UsersSettings oldSettings, UsersSettings newSettings, String user) throws AdminException {
    Map<String, User> oldUsers = oldSettings.getUsersMap();
    Map<String, User> newUsers = newSettings.getUsersMap();
    for (Map.Entry<String, User> e : oldUsers.entrySet()) {
      User oldUser = e.getValue();
      User newUser = newUsers.get(e.getKey());
      if (newUser == null) {
        j.addRecord(JournalRecord.Type.REMOVE, subject, user,"user_removed", oldUser.getLogin());
      } else {
        List<Method> getters = getGetters(User.class, "getPassword");
        List<Object> oldValues = callGetters(getters, oldUser);
        List<Object> newValues = callGetters(getters, newUser);
        logChanges(j, oldValues, newValues, getters, user, "user_property_changed", newUser.getLogin());
        if (!oldUser.getPassword().equals(newUser.getPassword())) {
          j.addRecord(JournalRecord.Type.CHANGE, subject, user, "user_property_changed", "password", "******", "******", newUser.getLogin());
        }
      }
    }
    for (Map.Entry<String, User> e : newUsers.entrySet()) {
      if (!oldUsers.containsKey(e.getKey())) {
        j.addRecord(JournalRecord.Type.ADD, subject, user,"user_added", e.getValue().getLogin());
      }
    }
  }
}
