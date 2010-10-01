package ru.novosoft.smsc.admin.profile;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.CCLookupProfileResult;
import ru.novosoft.smsc.admin.cluster_controller.CCProfile;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Address;

import java.io.File;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * @author Artem Snopkov
 */
public class ProfileManagerImpl implements ProfileManager {

  private final File profilesFile;
  private final FileSystem fs;
  private final ClusterController cc;
  private final boolean smsx;
  private final ReadWriteLock profilesLock = new ReentrantReadWriteLock();

  public ProfileManagerImpl(boolean smsx, File profilesFile, FileSystem fs, ClusterController cc) {
    this.smsx = smsx;
    this.profilesFile = profilesFile;
    this.cc = cc;
    this.fs = fs;
  }

  public ProfileLookupResult lookupProfile(Address mask) throws AdminException {
    if (!cc.isOnline())
      throw new ProfileException("profiler.offline");

    try {
      profilesLock.readLock().lock();

      CCLookupProfileResult lpr = cc.lookupProfile(mask);
      Profile p = new Profile();
      CCProfile profile = lpr.getProfile();

      p.setInputAccessMask(profile.getAccessMaskIn());
      p.setOutputAccessMask(profile.getAccessMaskOut());
      p.setGroupId(profile.getClosedGroupId());
      p.setDivert(profile.getDivert());
      p.setDivertActive(profile.isDivertActive());
      p.setDivertActiveAbsent(profile.isDivertActiveAbsent());
      p.setDivertActiveBarred(profile.isDivertActiveBarred());
      p.setDivertActiveBlocked(profile.isDivertActiveBlocked());
      p.setDivertActiveCapacity(profile.isDivertActiveCapacity());
      p.setDivertModifiable(profile.isDivertModifiable());
      switch (profile.getHide()) {
        case HideDisabled:
          p.setAliasHide(AliasHide.FALSE);
          break;
        case HideEnabled:
          p.setAliasHide(AliasHide.TRUE);
          break;
        default:
          p.setAliasHide(AliasHide.SUBSTITUTE);
      }
      p.setAliasModifiable(profile.isHideModifiable());
      p.setLatin1(profile.isLatin1());
      p.setLocale(profile.getLocale());
      p.setNick(profile.getNick());
      switch (profile.getReportOptions()) {
        case ReportNone:
          p.setReportOptions(ReportOption.NONE);
          break;
        case ReportFinal:
          p.setReportOptions(ReportOption.FINAL);
          break;
        default:
          p.setReportOptions(ReportOption.FULL);
      }
      p.setSponsored(profile.getSponsored());
      p.setSubscription(profile.getSubscription());
      p.setTranslit(profile.isTranslit());
      p.setUcs2(profile.isUcs2());
      p.setUdhConcat(profile.isUdhConcat());
      p.setUssd7bit(profile.isUssdIn7Bit());

      return new ProfileLookupResult(p, lpr.isExactMatch(), lpr.isMaskMatch(), lpr.isDefaultMatch());
    } finally {
      profilesLock.readLock().unlock();
    }
  }

  static CCProfile profile2CCProfile(Profile profile) {
    CCProfile p = new CCProfile();
    p.setAccessMaskIn(profile.getInputAccessMask());
    p.setAccessMaskOut(profile.getOutputAccessMask());
    p.setClosedGroupId(profile.getGroupId());
    p.setDivert(profile.getDivert());
    p.setDivertActive(profile.isDivertActive());
    p.setDivertActiveAbsent(profile.isDivertActiveAbsent());
    p.setDivertActiveBarred(profile.isDivertActiveBarred());
    p.setDivertActiveBlocked(profile.isDivertActiveBlocked());
    p.setDivertActiveCapacity(profile.isDivertActiveCapacity());
    p.setDivertModifiable(profile.isDivertModifiable());
    switch (profile.getAliasHide()) {
      case TRUE:
        p.setHide(CCProfile.HideOptions.HideEnabled);
        break;
      case FALSE:
        p.setHide(CCProfile.HideOptions.HideDisabled);
        break;
      default:
        p.setHide(CCProfile.HideOptions.HideSubstitute);
    }
    p.setHideModifiable(profile.isAliasModifiable());
    p.setLatin1(profile.isLatin1());

    p.setLocale(profile.getLocale());
    p.setNick(profile.getNick());
    switch (profile.getReportOptions()) {
      case NONE:
        p.setReportOptions(CCProfile.ReportOptions.ReportNone);
        break;
      case FINAL:
        p.setReportOptions(CCProfile.ReportOptions.ReportFinal);
        break;
      default:
        p.setReportOptions(CCProfile.ReportOptions.ReportFull);
    }
    p.setSponsored(profile.getSponsored());
    p.setSubscription(profile.getSubscription());
    p.setTranslit(profile.isTranslit());
    p.setUcs2(profile.isUcs2());
    p.setUdhConcat(profile.isUdhConcat());
    p.setUssdIn7Bit(profile.isUssd7bit());
    return p;
  }

  public void updateProfile(Profile profile) throws AdminException {
    if (!cc.isOnline())
      throw new ProfileException("profiler.offline");

    try {
      profilesLock.writeLock().lock();

      cc.updateProfile(profile.getAddress(), profile2CCProfile(profile));
    } finally {
      profilesLock.writeLock().unlock();
    }
  }

  public void deleteProfile(Address mask) throws AdminException {
    if (!cc.isOnline())
      throw new ProfileException("profiler.offline");

    try {
      profilesLock.writeLock().lock();
      cc.deleteProfile(mask);
    } finally {
      profilesLock.writeLock().unlock();
    }
  }


  public ProfilesSet getProfiles() throws AdminException {
    profilesLock.readLock().lock();
    if (cc.isOnline())
      cc.lockProfiles(false);

    return new ProfilesSetImpl(fs.getInputStream(profilesFile), smsx) {
      public void close() throws AdminException {
        super.close();
        profilesLock.readLock().unlock();
        if (cc.isOnline())
          cc.unlockProfiles();
      }
    };
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    if (!cc.isOnline())
      return null;

    ConfigState state = cc.getProfilesState();
    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();
    if (state != null) {
      long lastUpdate = state.getCcLastUpdateTime();
      for (Map.Entry<Integer, Long> e : state.getInstancesUpdateTimes().entrySet()) {
        SmscConfigurationStatus s = e.getValue() >= lastUpdate ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
        result.put(e.getKey(), s);
      }
    }
    return result;
  }


}
