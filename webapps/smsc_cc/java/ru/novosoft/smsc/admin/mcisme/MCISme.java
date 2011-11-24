package ru.novosoft.smsc.admin.mcisme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Address;

/**
 * User: artem
 * Date: 24.11.11
 */
public interface MCISme {
  Profile getProfile(Address subscr) throws AdminException;

  void saveProfile(Profile profile) throws AdminException;

  Statistics getStats() throws AdminException;

  RunStatistics getRunStats() throws AdminException;

  Schedule getSchedule(Address subscr) throws AdminException;

  void flushStats() throws AdminException;

  boolean isOnline() throws AdminException;   //todo Может убрать?
}
