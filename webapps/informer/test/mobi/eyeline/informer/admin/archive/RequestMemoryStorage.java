package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.AdminException;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
public class RequestMemoryStorage implements RequestStorage {

  private Map<Integer, Request> requests = new HashMap<Integer, Request>(10);

  @Override
  public Request getRequest(int requestId) throws AdminException {
    return requests.get(requestId);
  }

  @Override
  public void createRequest(Request request) throws AdminException {
    requests.put(request.getId(), request);
  }

  @Override
  public void changeStatus(int requestId, Request.Status status) throws AdminException {
    Request r = requests.get(requestId);
    if(r == null) {
      throw new ArchiveException("request_not_found", Integer.toString(requestId));
    }
    r.setStatus(status);
  }

  @Override
  public void setError(int requestId, String error, String ... args) throws AdminException {
    Request r = requests.get(requestId);
    if(r == null) {
      throw new ArchiveException("request_not_found", Integer.toString(requestId));
    }
    r.setError(error);
    r.setErrorArgs(args);
  }

  @Override
  public void rename(int requestId, String name) throws AdminException {
    Request r = requests.get(requestId);
    if(r == null) {
      throw new ArchiveException("request_not_found", Integer.toString(requestId));
    }
    r.setName(name);
  }

  @Override
  public void removeRequest(int requestId) throws AdminException {
    requests.remove(requestId);
  }

  @Override
  public List<Request> getRequests() throws AdminException {
    return new ArrayList<Request>(requests.values());
  }
}
