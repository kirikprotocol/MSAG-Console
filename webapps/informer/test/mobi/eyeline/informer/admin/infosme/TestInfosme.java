package mobi.eyeline.informer.admin.infosme;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.categories.Category;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.retry_policies.RetryPolicy;
import mobi.eyeline.informer.admin.smsc.Smsc;

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

  public void addSmsc(Smsc smsc) throws AdminException {
    if(smsc.getSmscId() == null) {
      throw new IllegalArgumentException("Id is null");
    }
    smscHelper.add(smsc.getSmscId());
  }

  public void removeSmsc(Smsc smsc) throws AdminException {
    if(smsc.getSmscId() == null) {
      throw new IllegalArgumentException("Id is null");
    }
    smscHelper.remove(smsc.getSmscId());
  }

  public void updateSmsc(Smsc smsc) throws AdminException {
    if(smsc.getSmscId() == null) {
      throw new IllegalArgumentException("Id is null");
    }
    smscHelper.update(smsc.getSmscId());
  }

  public void addRegion(Region region) throws AdminException {
    if(region.getRegionId() == null) {
      throw new IllegalArgumentException("Id is null");
    }
    regionHelper.add(region.getRegionId());
  }

  public void updateRegion(Region region) throws AdminException {
    if(region.getRegionId() == null) {
      throw new IllegalArgumentException("Id is null");
    }
    regionHelper.update(region.getRegionId());
  }

  public void removeRegion(Region region) throws AdminException {
    if(region.getRegionId() == null) {
      throw new IllegalArgumentException("Id is null");
    }
    regionHelper.remove(region.getRegionId());
  }

  public void addRetryPolicy(RetryPolicy policy) throws AdminException {
    if(policy.getPolicyId() == null) {
      throw new IllegalArgumentException("Id is null");
    }
    retryHelper.add(policy.getPolicyId());
  }

  public void updateRetryPolicy(RetryPolicy policy) throws AdminException {
    if(policy.getPolicyId() == null) {
      throw new IllegalArgumentException("Id is null");
    }
    retryHelper.update(policy.getPolicyId());
  }

  public void removeRetryPolicy(RetryPolicy policy) throws AdminException {
    if(policy.getPolicyId() == null) {
      throw new IllegalArgumentException("Id is null");
    }
    retryHelper.remove(policy.getPolicyId());
  }

  public void setDefaultSmsc(Smsc smsc) throws AdminException {
    if(smsc.getSmscId() == null) {
      throw new IllegalArgumentException("Id is null");
    }
    smscHelper.setDefaultSmsc(smsc.getSmscId());
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

  public boolean isOnline() throws AdminException {
    return true;
  }
}
