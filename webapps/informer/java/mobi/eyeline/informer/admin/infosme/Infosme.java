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
public interface Infosme {

  public void addSmsc(Smsc smsc) throws AdminException;

  public void removeSmsc(Smsc smsc) throws AdminException;

  public void updateSmsc(Smsc smsc) throws AdminException;

  public void addRegion(Region region) throws AdminException;

  public void updateRegion(Region region) throws AdminException;

  public void removeRegion(Region region) throws AdminException;

  public void addRetryPolicy(RetryPolicy policy) throws AdminException;

  public void updateRetryPolicy(RetryPolicy policyI) throws AdminException;

  public void removeRetryPolicy(RetryPolicy policy) throws AdminException;

  public void setDefaultSmsc(Smsc smsc) throws AdminException;

  public void setCategories(List<Category> categories) throws AdminException;

  public List<Category> getCategories() throws AdminException;

  public boolean isOnline() throws AdminException;

}
