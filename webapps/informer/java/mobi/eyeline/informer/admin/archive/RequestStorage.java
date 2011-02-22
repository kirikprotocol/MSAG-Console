package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.AdminException;

import java.util.Collection;
import java.util.Date;


/**
 * @author Aleksandr Khalitov
 */
public interface RequestStorage {

  public Request getRequest(int requestId) throws AdminException;

  public void createRequest(Request request) throws AdminException;

  public void changeStatus(int requestId, Request.Status status) throws AdminException;

  public void setEndDate(int requestId, Date date) throws AdminException;

  public void rename(int requestId, String name) throws AdminException;

  public void removeRequest(int requestId) throws AdminException;

  public Collection<Request> getRequests() throws AdminException;

}
