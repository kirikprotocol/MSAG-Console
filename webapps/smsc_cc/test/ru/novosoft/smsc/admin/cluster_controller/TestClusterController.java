package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.alias.TestAliasManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Address;

import java.io.File;
import java.util.Collection;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class TestClusterController extends TestClusterControllerStub {

  private final TestAclHelper aclHelper = new TestAclHelper();
  private final TestProfilesHelper profilesHelper;
  private final TestLoggersHelper loggersHelper = new TestLoggersHelper();

  private final int smscInstancesNumber;
  private final File aliasesFile;
  private final File mscsFile;


  private long aclLastUpdateTime = System.currentTimeMillis();

  public TestClusterController(File aliasesFile, File mscsFile, File profilesFile, boolean smsx, int version, FileSystem fs, int smscInstancesNumber) throws AdminException {
    super(smscInstancesNumber);
    this.smscInstancesNumber = smscInstancesNumber;
    this.aliasesFile = aliasesFile;
    this.mscsFile = mscsFile;
    this.profilesHelper = new TestProfilesHelper(profilesFile, fs, smsx, version);
  }


  public void shutdown() {

  }

  // ALIASES ===========================================================================================================

  public void addAlias(Address address, Address alias, boolean aliasHide) throws AdminException {
    TestAliasManager.helpAddAlias(aliasesFile, address, alias, aliasHide);
    aliasLastUpdateTime = System.currentTimeMillis();
  }

  public void delAlias(Address alias) throws AdminException {
    TestAliasManager.helpRemoveAlias(aliasesFile, alias);
    aliasLastUpdateTime = System.currentTimeMillis();
  }

  // ACCESS CONTROL LIST ===============================================================================================

  public List<CCAclInfo> getAcls() throws AdminException {
    return aclHelper.getAcls();
  }

  public List<Address> getAclAddresses(int aclId) throws AdminException {
    return aclHelper.getAclAddresses(aclId);
  }

  public void createAlc(int aclId, String name, String description, List<Address> addresses) throws AdminException {
    aclLastUpdateTime = System.currentTimeMillis();
    aclHelper.createAcl(aclId, name, description, addresses);
  }

  public void updateAcl(CCAclInfo acl) throws AdminException {
    aclLastUpdateTime = System.currentTimeMillis();
    aclHelper.updateAcl(acl);
  }

  public void removeAcl(int aclId) throws AdminException {
    aclLastUpdateTime = System.currentTimeMillis();
    aclHelper.removeAcl(aclId);
  }

  public void addAddressesToAcl(int aclId, List<Address> addresses) throws AdminException {
    aclLastUpdateTime = System.currentTimeMillis();
    aclHelper.addAddresses(aclId, addresses);
  }

  public void removeAddressesFromAcl(int aclId, List<Address> addresses) throws AdminException {
    aclLastUpdateTime = System.currentTimeMillis();
    aclHelper.removeAddresses(aclId, addresses);
  }

  public CCAclInfo getAcl(int aclId) throws AdminException {
    return aclHelper.getAcl(aclId);
  }

  // PROFILES ================================================================================================

  public CCLookupProfileResult lookupProfile(Address address) throws AdminException {
    return profilesHelper.lookupProfile(address);
  }

  public void updateProfile(Address address, CCProfile profile) throws AdminException {
    profilesHelper.updateProfile(address, profile);
    lastProfilesUpdateTime = System.currentTimeMillis();
  }

  public void deleteProfile(Address address) throws AdminException {
    profilesHelper.deleteProfile(address);
    lastProfilesUpdateTime = System.currentTimeMillis();
  }

  //SMS

  @Override
  public void cancelSMS(String[] ids) throws AdminException {
    if(ids == null) {
      return;
    }
    for(String s : ids) {
      System.out.println("Cancel sms: "+s);
    }
  }


  // LOGGING =================================================================================================


  @Override
  public Collection<CCLoggingInfo> getLoggers() throws AdminException {
    return loggersHelper.getLoggers();
  }

  @Override
  public void setLoggers(Collection<CCLoggingInfo> loggers) throws AdminException {
    loggersHelper.setLoggers(loggers);
  }
}
