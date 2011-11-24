package ru.novosoft.smsc.web.config.mcisme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.mcisme.*;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.journal.Journal;

/**
 * User: artem
 * Date: 24.11.11
 */
public class WMCISme implements MCISme {

  private final MCISme wrapped;
  private final Journal j;
  private final String user;

  public WMCISme(MCISme wrapped, Journal j, String user) {
    this.wrapped = wrapped;
    this.j = j;
    this.user = user;
  }

  public Profile getProfile(Address subscr) throws AdminException {
    return wrapped.getProfile(subscr);
  }

  public void saveProfile(Profile profile) throws AdminException {
    wrapped.saveProfile(profile);
    j.user(user).change("update.values",
        profile.getSubscriber().getSimpleAddress(),
        profile.isInform() + "",
        profile.isNotify() + "",
        profile.isWantNotifyMe() + "",
        profile.isAbsent() + "",
        profile.isBusy() + "",
        profile.isNoReplay() + "",
        profile.isUnconditional() + "",
        profile.isDetach() + ""
        ).mcismeProfile();
  }

  public Statistics getStats() throws AdminException {
    return wrapped.getStats();
  }

  public RunStatistics getRunStats() throws AdminException {
    return wrapped.getRunStats();
  }

  public Schedule getSchedule(Address subscr) throws AdminException {
    return wrapped.getSchedule(subscr);
  }

  public void flushStats() throws AdminException {
    wrapped.flushStats();
  }

  public boolean isOnline() throws AdminException {
    return wrapped.isOnline();
  }
}
