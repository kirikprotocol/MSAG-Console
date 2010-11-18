package mobi.eyeline.informer.admin.infosme;

import mobi.eyeline.informer.admin.AdminException;


/**
 * Администрирование Informer
 *
 * @author Aleksandr Khalitov
 */
public interface Infosme {

  public void addSmsc(String smscId) throws AdminException;

  public void removeSmsc(String smscId) throws AdminException;

  public void updateSmsc(String smscId) throws AdminException;

  public void addRegion(int regionId) throws AdminException;

  public void updateRegion(int regionId) throws AdminException;

  public void removeRegion(int regionId) throws AdminException;

  public void addUser(String userId) throws AdminException;

  public void updateUser(String userId) throws AdminException;

  public void removeUser(String userId) throws AdminException;

  public void setDefaultSmsc(String smscId) throws AdminException;

  public void sendTestSms(TestSms sms) throws AdminException;

  public boolean isOnline() throws AdminException;

  public void shutdown();

}
