package mobi.eyeline.informer.admin.infosme;

import mobi.eyeline.informer.admin.AdminException;

import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class TestInfosme implements Infosme{

  private TestSmscHelper smscHelper;

  private TestRegionsHelper regionHelper;

  private TestRetryPoliciesHelper retryHelper;

  private TestCategoriesHelper categoriesHelper;

  public TestInfosme() {
    this.smscHelper = new TestSmscHelper();
    this.regionHelper = new TestRegionsHelper();
    this.retryHelper = new TestRetryPoliciesHelper();
    this.categoriesHelper = new TestCategoriesHelper();
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

  public void addRegion(String regionId) throws AdminException {
    if(regionId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    regionHelper.add(regionId);
  }

  public void updateRegion(String regionId) throws AdminException {
    if(regionId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    regionHelper.update(regionId);
  }

  public void removeRegion(String regionId) throws AdminException {
    if(regionId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    regionHelper.remove(regionId);
  }

  public void addRetryPolicy(String policyId) throws AdminException {
    if(policyId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    retryHelper.add(policyId);
  }

  public void updateRetryPolicy(String policyId) throws AdminException {
    if(policyId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    retryHelper.update(policyId);
  }

  public void removeRetryPolicy(String policyId) throws AdminException {
    if(policyId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    retryHelper.remove(policyId);
  }

  public void setDefaultSmsc(String smscId) throws AdminException {
    if(smscId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    smscHelper.setDefaultSmsc(smscId);
  }

  public void setCategories(List<Category> categories) throws AdminException {
    if(categories == null) {
      throw new IllegalArgumentException("Categories is null");
    }
    categoriesHelper.setCategories(categories);  
  }

  public List<Category> getCategories() throws AdminException {
    return categoriesHelper.getCategories();
  }

  private boolean online = true;

  public boolean isOnline() throws AdminException {
    return online;
  }

  public void shutdown() {
    online = false;
  }
}
