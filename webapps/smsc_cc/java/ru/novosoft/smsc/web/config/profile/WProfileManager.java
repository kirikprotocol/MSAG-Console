package ru.novosoft.smsc.web.config.profile;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.profile.Profile;
import ru.novosoft.smsc.admin.profile.ProfileLookupResult;
import ru.novosoft.smsc.admin.profile.ProfileManager;
import ru.novosoft.smsc.admin.profile.ProfilesSet;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.Map;

import static ru.novosoft.smsc.web.config.DiffHelper.*;

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

  public Profile getDefaultProfile() throws AdminException {
    return wrapped.getDefaultProfile();
  }

  public void updateDefaultProfile(Profile profile) throws AdminException {
    Profile oldProfile = getDefaultProfile();
    wrapped.updateDefaultProfile(profile);
    logChanges(oldProfile, profile);
  }

  public ProfileLookupResult lookupProfile(Address mask) throws AdminException {
    return wrapped.lookupProfile(mask);
  }

  public void updateProfile(final Profile profile) throws AdminException {
    ProfileLookupResult oldProfileRes = lookupProfile(profile.getAddress());
    Profile oldProfile = oldProfileRes.getProfile();
    wrapped.updateProfile(profile);

    if (oldProfileRes.isExactMatch()) {
      logChanges(oldProfile, profile);
    } else {
      j.user(user).add().profile(profile.getAddress().getNormalizedAddress());
    }
  }

  private void logChanges(final Profile oldProfile, final Profile profile) {
    findChanges(oldProfile, profile, Profile.class, new ChangeListener() {
        public void foundChange(String propertyName, Object oldValue, Object newValue) {
          j.user(user).change("change_property", propertyName, valueToString(oldValue), valueToString(newValue)).profile(profile.getAddress().getNormalizedAddress());
        }
      }, "address");
  }

  public void deleteProfile(Address mask) throws AdminException {
    wrapped.deleteProfile(mask);
    j.user(user).remove().profile(mask.getNormalizedAddress());
  }

  public ProfilesSet getProfiles() throws AdminException {
    return wrapped.getProfiles();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    return wrapped.getStatusForSmscs();
  }
}
