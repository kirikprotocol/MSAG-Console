package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.users.User;

import java.lang.reflect.Method;
import java.util.List;

/**
 * Ищет и журналирует изменения в настройках пользователя
 *
 * @author Aleksandr Khalitov
 */
class UserSettingsDiffHelper extends DiffHelper {

  public UserSettingsDiffHelper() {
    super(Subject.USERS);
  }

  public void logChanges(User oldUser, User newUser, Journal journal, String user) throws AdminException {
    List<Method> getters = getGetters(User.class, "getPassword");
    List<Object> oldValues = callGetters(getters, oldUser);
    List<Object> newValues = callGetters(getters, newUser);
    logChanges(journal, oldValues, newValues, getters, user, "user_property_changed", newUser.getLogin());
    if (!oldUser.getPassword().equals(newUser.getPassword())) {
      journal.addRecord(JournalRecord.Type.CHANGE, subject, user, "user_property_changed", "password", "******", "******", newUser.getLogin());
    }

  }

  public void logAddUser(String login, Journal journal, String user) throws AdminException {
    journal.addRecord(JournalRecord.Type.ADD, subject, user, "user_added", login);
  }

  public void logRemoveUser(String login, Journal journal, String user) throws AdminException {
    journal.addRecord(JournalRecord.Type.REMOVE, subject, user, "user_removed", login);
  }
}
