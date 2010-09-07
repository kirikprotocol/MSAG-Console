package ru.novosoft.smsc.admin.cluster_controller;

import com.eyeline.utils.tree.radix.TemplatesRTree;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.profile.TestProfileManager;
import ru.novosoft.smsc.util.Address;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class TestProfilesHelper {

  private final TemplatesRTree<CCProfileWithAddress> profilesTree = new TemplatesRTree<CCProfileWithAddress>();
  private final File profilesFile;
  private final FileSystem fileSystem;
  private final boolean smsx;
  private final int version;

  public TestProfilesHelper(File profilesFile, FileSystem fileSystem, boolean smsx, int version) throws AdminException {
    this.profilesFile = profilesFile;
    this.fileSystem = fileSystem;
    this.smsx = smsx;
    this.version = version;

    try {
      Map<Address, CCProfile> profiles = TestProfileManager.loadProfiles(fileSystem.getInputStream(profilesFile), smsx);
      for (Map.Entry<Address, CCProfile> e : profiles.entrySet()) {
        profilesTree.put(e.getKey().getSimpleAddress(), new CCProfileWithAddress(e.getValue(), e.getKey()));
      }
      if (!profilesTree.containsKey(".5.0.DEFAULT"))
        profilesTree.put(".5.0.DEFAULT", new CCProfileWithAddress(new CCProfile(), new Address(".5.0.DEFAULT")));
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  public CCLookupProfileResult lookupProfile(Address address) {
    CCProfileWithAddress profile = profilesTree.get(address.getSimpleAddress());
    if (profile == null)
      profile = profilesTree.get(".5.0.DEFAULT");

    boolean exactMatch = profile.address.equals(address);
    boolean defaultMatch = profile.address.equals(new Address(5,0,"DEFAULT"));
    boolean maskMatch = !exactMatch && !defaultMatch;

    return new CCLookupProfileResult(profile.profile, exactMatch, maskMatch, defaultMatch);
  }

  private void saveProfiles() throws AdminException {
    Map<Address, CCProfile> profiles = new HashMap<Address, CCProfile>();
    for (String key : profilesTree.keys()) {
      CCProfileWithAddress p = profilesTree.get(key);
      profiles.put(p.address, p.profile);
    }

    try {
      TestProfileManager.saveProfiles(profiles, fileSystem.getOutputStream(profilesFile), smsx, version);
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  public void updateProfile(Address address, CCProfile profile) throws AdminException {
    profilesTree.put(address.getSimpleAddress(), new CCProfileWithAddress(profile, address));

    saveProfiles();
  }

  public void deleteProfile(Address address) throws AdminException {
    profilesTree.remove(address.getSimpleAddress());

    saveProfiles();
  }

  private class CCProfileWithAddress {
    CCProfile profile;
    Address address;

    private CCProfileWithAddress(CCProfile profile, Address address) {
      this.profile = profile;
      this.address = address;
    }
  }
}
