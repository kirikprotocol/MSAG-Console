package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.AdminException;

import java.util.List;


/**
 * @author Aleksandr Khalitov
 */
public interface RequestStorage {

  public Request getRequest(int requestId) throws AdminException;

  public void createRequest(Request request) throws AdminException;

  public void changeStatus(int requestId, Request.Status status) throws AdminException;

  public void setError(int requestId, String error) throws AdminException;

  public void rename(int requestId, String name) throws AdminException;

  public void removeRequest(int requestId) throws AdminException;

  public List<Request> getRequests() throws AdminException;

}
