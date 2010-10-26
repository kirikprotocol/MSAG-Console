package mobi.eyeline.informer.admin.infosme;

import mobi.eyeline.informer.admin.AdminException;

import java.util.List;


/**
 * Администрирование Informer
 * @author Aleksandr Khalitov
 */
public interface Infosme {

  public void addSmsc(String smscId) throws AdminException;

  public void removeSmsc(String smscId) throws AdminException;

  public void updateSmsc(String smscId) throws AdminException;

  public void addRegion(String regionId) throws AdminException;

  public void updateRegion(String regionId) throws AdminException;

  public void removeRegion(String regionId) throws AdminException;

  public void addUser(String userId) throws AdminException;

  public void updateUser(String userId) throws AdminException;

  public void removeUser(String userId) throws AdminException;

  public void setDefaultSmsc(String smscId) throws AdminException;

  public void setCategories(List<Category> categories) throws AdminException;

  public List<Category> getCategories() throws AdminException;

  public boolean isOnline() throws AdminException;

  public void shutdown();

}
