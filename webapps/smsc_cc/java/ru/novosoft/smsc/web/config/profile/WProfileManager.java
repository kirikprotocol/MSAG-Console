package ru.novosoft.smsc.web.config.profile;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonSettings;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.profile.Profile;
import ru.novosoft.smsc.admin.profile.ProfileLookupResult;
import ru.novosoft.smsc.admin.profile.ProfileManager;
import ru.novosoft.smsc.admin.profile.ProfilesSet;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.config.DiffHelper;
import ru.novosoft.smsc.web.journal.Journal;
import ru.novosoft.smsc.web.journal.JournalRecord;

import java.lang.reflect.Method;
import java.util.List;
import java.util.Map;

/**
 * @author Artem Snopkov
 */

public class WProfileManager implements ProfileManager {

  private final ProfileManager wrapped;
  private final Journal j;
  private final String user;

  public WProfileManager(ProfileManager wrapped, Journal j, String user) {
    this.wrapped = wrapped;
    this.j = j;
    this.user = user;
  }

  public ProfileLookupResult lookupProfile(Address mask) throws AdminException {
    return wrapped.lookupProfile(mask);
  }

  public void updateProfile(Profile profile) throws AdminException {
    ProfileLookupResult oldProfileRes = lookupProfile(profile.getAddress());
    Profile oldProfile = oldProfileRes.getProfile();
    wrapped.updateProfile(profile);
    
    List<Method> getters = DiffHelper.getGetters(Profile.class);
    List<Object> oldValues = DiffHelper.callGetters(getters, oldProfile);
    List<Object> newValues = DiffHelper.callGetters(getters, profile);
    DiffHelper.logChanges(j, JournalRecord.Subject.PROFILE, oldValues, newValues, getters, user);
  }

  public void deleteProfile(Address mask) throws AdminException {
    wrapped.deleteProfile(mask);
    JournalRecord r = j.addRecord(JournalRecord.Type.REMOVE, JournalRecord.Subject.PROFILE, user);
    r.setDescription("profile.deleted", mask.getNormalizedAddress());
  }

  public ProfilesSet getProfiles() throws AdminException {
    return wrapped.getProfiles();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    return wrapped.getStatusForSmscs();
  }
}
