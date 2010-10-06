package ru.novosoft.smsc.admin.users;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public interface UsersManager {
  UsersSettings getUsersSettings() throws AdminException;

  void updateSettings(UsersSettings usersSettings) throws AdminException;
}
