package ru.novosoft.smsc.web.journal;

import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.admin.users.UsersSettings;

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

  public void logChanges(Journal j, UsersSettings oldSettings, UsersSettings newSettings, String user) {
    Map<String, User> oldUsers = oldSettings.getUsersMap();
    Map<String, User> newUsers = newSettings.getUsersMap();
    for (Map.Entry<String, User> e : oldUsers.entrySet()) {
      User oldUser = e.getValue();
      User newUser = newUsers.get(e.getKey());
      if (newUser == null) {
        j.addRecord(JournalRecord.Type.REMOVE, subject, user).setDescription("user_removed", oldUser.getLogin());
      } else {
        List<Method> getters = getGetters(User.class, "getPassword");
        List<Object> oldValues = callGetters(getters, oldUser);
        List<Object> newValues = callGetters(getters, newUser);
        logChanges(j, oldValues, newValues, getters, user, "user_property_changed", newUser.getLogin());
        if (!oldUser.getPassword().equals(newUser.getPassword())) {
          j.addRecord(JournalRecord.Type.CHANGE, subject, user).setDescription("user_property_changed", "password", "******", "******", newUser.getLogin());
        }
      }
    }
    for (Map.Entry<String, User> e : newUsers.entrySet()) {
      if (!oldUsers.containsKey(e.getKey())) {
        j.addRecord(JournalRecord.Type.ADD, subject, user).setDescription("user_added", e.getValue().getLogin());
      }
    }
  }
}
