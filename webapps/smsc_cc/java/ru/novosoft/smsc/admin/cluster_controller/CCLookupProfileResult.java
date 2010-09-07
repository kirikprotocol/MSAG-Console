package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.cluster_controller.protocol.Profile;
import ru.novosoft.smsc.admin.cluster_controller.protocol.ProfileMatchType;

/**
 * @author Artem Snopkov
 */
public class CCLookupProfileResult {
  private final CCProfile profile;
  private final boolean exactMatch;
  private final boolean maskMatch;
  private final boolean defaultMatch;

  CCLookupProfileResult(Profile profile, ProfileMatchType mt) {
    this.profile = new CCProfile(profile);
    this.exactMatch = mt == ProfileMatchType.Exact;
    this.maskMatch = mt == ProfileMatchType.Mask;
    this.defaultMatch = mt == ProfileMatchType.Default;
  }

  public CCLookupProfileResult(CCProfile profile, boolean exactMatch, boolean maskMatch, boolean defaultMatch) {
    this.profile = profile;
    this.exactMatch = exactMatch;
    this.maskMatch = maskMatch;
    this.defaultMatch = defaultMatch;
  }

  public CCProfile getProfile() {
    return profile;
  }

  public boolean isExactMatch() {
    return exactMatch;
  }

  public boolean isMaskMatch() {
    return maskMatch;
  }

  public boolean isDefaultMatch() {
    return defaultMatch;
  }
}
