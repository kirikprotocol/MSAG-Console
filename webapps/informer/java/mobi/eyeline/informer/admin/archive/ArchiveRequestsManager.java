package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.delivery.Visitor;

import java.io.File;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * @author Aleksandr Khalitov
 */
public class ArchiveRequestsManager {

  private final AtomicInteger lastId;

  private final RequestProcessor processor;

  private final RequestStorage storage;

  private final ResultsManager resultsManager;

  private int getNextId() {
    return lastId.incrementAndGet();
  }

  public ArchiveRequestsManager(ArchiveContext context, ArchiveRequestSettings requestSettings) throws InitException {
    this(new RequestFileStorage(new File(requestSettings.getRequestsDir()), context.getFileSystem()),
        context, new File(requestSettings.getResultsDir()), requestSettings.getChunkSize(), requestSettings.getExecutorsSize());
  }

  ArchiveRequestsManager(RequestStorage storage, ArchiveContext context, File resultDir, int chunkSize, int executorSize) throws InitException {
    this.storage = storage;
    this.resultsManager = new ResultsManager(resultDir, context.getFileSystem());
    int lastId = 0;
    this.processor = new RequestProcessor(context, storage, resultsManager, chunkSize, executorSize);
    try{
      for(Request r : storage.getRequests()) {
        if (r.getId() > lastId) {
          lastId = r.getId();
        }
        if(r.getStatus() == Request.Status.IN_PROCESS) {
          processor.execute(r);
        }
      }
      this.lastId = new AtomicInteger(lastId);
    }catch (AdminException e) {
      throw new InitException(e);
    }
  }

  public DeliveriesRequest createRequest(String login, DeliveriesRequestPrototype request) throws AdminException {
    DeliveriesRequest deliveriesRequest = new DeliveriesRequest();
    deliveriesRequest.copyFrom(request);
    deliveriesRequest.setCreater(login);
    deliveriesRequest.setId(getNextId());
    deliveriesRequest.validate();

    storage.createRequest(deliveriesRequest);

    processor.execute(deliveriesRequest);

    return new DeliveriesRequest(deliveriesRequest);
  }

  public MessagesRequest createRequest(String login, MessagesRequestPrototype _request) throws AdminException {
    MessagesRequest request = new MessagesRequest();
    request.copyFrom(_request);
    request.setCreater(login);
    request.setId(getNextId());
    request.validate();

    storage.createRequest(request);

    processor.execute(request);

    return request;
  }

  public Request getRequest(int requestId) throws AdminException {
    Request q = storage.getRequest(requestId);
    if(q == null) {
      return null;
    }
    setStatus(q);
    return q;
  }

  private void setStatus(Request q) {
    if(q.getStatus() != Request.Status.CANCELED && q.getStatus() != Request.Status.FINISHED) {
      q.setStatus(Request.Status.IN_PROCESS);
      Integer proggress = processor.getProgress(q.getId());
      if(proggress == null) {
        proggress = 100;
      }
      q.setProgress(proggress);
    }else {
      q.setProgress(100);
    }

  }

  public List<Request> getRequests() throws AdminException {
    List<Request> c = storage.getRequests();
    for(Request r : c) {
      setStatus(r);
    }
    return c;
  }

  public void modifyRequest(Request request) throws AdminException {
    Request q = storage.getRequest(request.getId());
    if(q == null) {
      throw new ArchiveException("request_not_found", Integer.toString(request.getId()));
    }
    storage.rename(request.getId(), request.getName());
  }

  public void getDeliveriesResult(int requestId, Visitor<ArchiveDelivery> visitor) throws AdminException {
    resultsManager.getDeliveriesResults(requestId, visitor);
  }

  public void getMessagesResult(int requestId, Visitor<ArchiveMessage> visitor) throws AdminException {
    resultsManager.getMessagesResults(requestId, visitor);
  }

  public void removeRequest(int requestId) throws AdminException {
    Request r = storage.getRequest(requestId);
    if(r != null) {
      processor.cancel(requestId);
      storage.removeRequest(requestId);
      switch (r.getType()) {
        case messages:   resultsManager.removeMessagesResult(requestId); break;
        case deliveries: resultsManager.removeDeliveriesResult(requestId); break;
      }
    }
  }

  public void cancelRequest(int requestId) throws AdminException {
    Request request = storage.getRequest(requestId);
    if(request == null) {
      throw new ArchiveException("request_not_found", Integer.toString(requestId));
    }
    processor.cancel(requestId);
    storage.changeStatus(requestId, Request.Status.CANCELED);
  }

  public void shutdown() {
    if(processor != null) {
      processor.shutdown();
    }
  }

}
