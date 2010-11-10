package mobi.eyeline.informer.admin.infosme;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
public class TestInfosme implements Infosme{

  private TestSmscHelper smscHelper;

  private TestRegionsHelper regionHelper;

  private TestUsersHelper usersHelper;

  public TestInfosme() {
    this.smscHelper = new TestSmscHelper();
    this.regionHelper = new TestRegionsHelper();
    this.usersHelper = new TestUsersHelper();
  }

  public void addSmsc(String smscId) throws AdminException {
    if(smscId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    smscHelper.add(smscId);
  }

  public void removeSmsc(String smscId) throws AdminException {
    if(smscId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    smscHelper.remove(smscId);
  }

  public void updateSmsc(String smscId) throws AdminException {
    if(smscId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    smscHelper.update(smscId);
  }

  public void addRegion(int regionId) throws AdminException {
    regionHelper.add(Integer.toString(regionId));
  }

  public void updateRegion(int regionId) throws AdminException {
    regionHelper.update(Integer.toString(regionId));
  }

  public void removeRegion(int regionId) throws AdminException {
    regionHelper.remove(Integer.toString(regionId));
  }

  public void addUser(String userId) throws AdminException {
    if(userId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    usersHelper.add(userId);
  }

  public void updateUser(String userId) throws AdminException {
    if(userId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    usersHelper.update(userId);
  }

  public void removeUser(String userId) throws AdminException {
    if(userId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    usersHelper.remove(userId);
  }


  public void setDefaultSmsc(String smscId) throws AdminException {
    if(smscId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    smscHelper.setDefaultSmsc(smscId);
  }

  public void sendTestSms(TestSms sms) throws AdminException {
    if(sms == null) {
      throw new IllegalArgumentException("sms is null");
    }
    System.out.println("Sms has been sent: "+sms);
  }

  private boolean online = true;

  public boolean isOnline() throws AdminException {
    return online;
  }

  public void shutdown() {
    online = false;
  }
}
