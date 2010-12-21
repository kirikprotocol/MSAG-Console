package mobi.eyeline.informer.admin.infosme;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
public class TestInfosme implements Infosme{

  private TestSmscHelper smscHelper;

  private TestRegionsHelper regionHelper;

  private TestUsersHelper usersHelper;

  private boolean errorMode;

  public TestInfosme() {
    this.smscHelper = new TestSmscHelper();
    this.regionHelper = new TestRegionsHelper();
    this.usersHelper = new TestUsersHelper();
  }

  public TestInfosme (boolean errorMode) {
    this();
    this.errorMode = errorMode;
  }

  public void addSmsc(String smscId) throws AdminException {
    if(smscId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    if (errorMode)
      throw new InfosmeException("interaction_error", "999");
    smscHelper.add(smscId);
  }

  public void removeSmsc(String smscId) throws AdminException {
    if(smscId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    if (errorMode)
      throw new InfosmeException("interaction_error", "999");
    smscHelper.remove(smscId);
  }

  public void updateSmsc(String smscId) throws AdminException {
    if(smscId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    if (errorMode)
      throw new InfosmeException("interaction_error", "999");
    smscHelper.update(smscId);
  }

  public void addRegion(int regionId) throws AdminException {
    if (errorMode)
      throw new InfosmeException("interaction_error", "999");
    regionHelper.add(Integer.toString(regionId));
  }

  public void updateRegion(int regionId) throws AdminException {
    if (errorMode)
      throw new InfosmeException("interaction_error", "999");
    regionHelper.update(Integer.toString(regionId));
  }

  public void removeRegion(int regionId) throws AdminException {
    if (errorMode)
      throw new InfosmeException("interaction_error", "999");
    regionHelper.remove(Integer.toString(regionId));
  }

  public void addUser(String userId) throws AdminException {
    if(userId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    if (errorMode)
      throw new InfosmeException("interaction_error", "999");
    usersHelper.add(userId);
  }

  public void updateUser(String userId) throws AdminException {
    if(userId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    if (errorMode)
      throw new InfosmeException("interaction_error", "999");
    usersHelper.update(userId);
  }

  public void removeUser(String userId) throws AdminException {
    if(userId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    if (errorMode)
      throw new InfosmeException("interaction_error", "999");
    usersHelper.remove(userId);
  }


  public void setDefaultSmsc(String smscId) throws AdminException {
    if(smscId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    if (errorMode)
      throw new InfosmeException("interaction_error", "999");
    smscHelper.setDefaultSmsc(smscId);
  }

  public void sendTestSms(TestSms sms) throws AdminException {
    if(sms == null) {
      throw new IllegalArgumentException("sms is null");
    }
    if (errorMode)
      throw new InfosmeException("interaction_error", "999");
    System.out.println("Sms has been sent: "+sms);
  }

  public void shutdown() {
  }
}
