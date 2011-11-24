package ru.novosoft.smsc.admin.mcisme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Address;

import java.util.HashMap;
import java.util.Map;

/**
 * author: Aleksandr Khalitov
 */
public class TestMCISme implements MCISme {

  private Map<String, Profile> profiles = new HashMap<String, Profile>();

  public TestMCISme() {
  }

  public Profile getProfile(Address subscr) throws AdminException {
    return profiles.get(subscr.getSimpleAddress());
  }

  public void saveProfile(Profile profile) throws AdminException {
    profiles.put(profile.getSubscriber().getSimpleAddress(), profile);
  }

  public Statistics getStats() throws AdminException {
    return null;
  }

  public RunStatistics getRunStats() throws AdminException {
    return null;
  }

  public Schedule getSchedule(Address subscr) throws AdminException {
    return null;
  }

  public void flushStats() throws AdminException {

  }

  public boolean isOnline() throws AdminException {
    return true;
  }
}
