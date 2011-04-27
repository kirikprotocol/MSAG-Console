package ru.sibinco.sponsored.stats.backend;

import org.apache.log4j.Category;

import java.io.File;
import java.util.Iterator;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class StatRequestManager {

  private static final Category logger = Category.getInstance(StatRequestManager.class);

  private int lastId;

  private final RequestProcessor processor;

  private final RequestStorage storage;

  private final ResultsManager resultsManager;

  private synchronized int getNextId() {
    return ++lastId;
  }

  public StatRequestManager(File requestsDir, File sponsArtefacts) throws StatisticsException {
    this.storage = new RequestStorage(requestsDir);
    this.resultsManager = new ResultsManager(requestsDir);
    this.processor = new RequestProcessor(storage, sponsArtefacts, new ResultsManager(requestsDir));

    Iterator i = storage.getRequests().iterator();
    while(i.hasNext()) {
      SponsoredRequest r = (SponsoredRequest)i.next();
      if (r.getId() > lastId) {
        lastId = r.getId();
      }
      if(r.getStatus() == SponsoredRequest.Status.IN_PROCESS) {
        processor.execute(r);
      }
    }
  }


  public SponsoredRequest createRequest(SponsoredRequestPrototype request) throws StatisticsException {
    SponsoredRequest sponsoredRequest = new SponsoredRequest();
    sponsoredRequest.copyFrom(request);
    sponsoredRequest.validate();
    sponsoredRequest.setId(getNextId());

    if(logger.isDebugEnabled()) {
      logger.debug("Create request: "+request);
    }

    storage.createRequest(sponsoredRequest);

    processor.execute(sponsoredRequest);

    return sponsoredRequest.copy();
  }



  public SponsoredRequest getRequest(int requestId) {
    SponsoredRequest q = storage.getRequest(requestId);
    if(q == null) {
      return null;
    }
    setStatus(q);
    return q;
  }

  private void setStatus(SponsoredRequest q) {
    if(q.getStatus() != SponsoredRequest.Status.CANCELED && q.getStatus() != SponsoredRequest.Status.READY && q.getStatus() != SponsoredRequest.Status.ERROR) {
      q.setStatus(SponsoredRequest.Status.IN_PROCESS);
      Integer proggress = processor.getProgress(q.getId());
      if(proggress == null) {
        q.setProgress(100);
      }else {
        q.setProgress(proggress.intValue());
      }
    }else {
      q.setProgress(100);
    }
  }


  public List getSponsoreRequests() {
    List c = storage.getSponsoredRequests();
    Iterator i = c.iterator();
    while(i.hasNext()) {
      setStatus((SponsoredRequest)i.next());
    }
    return c;
  }



  public void removeRequest(int requestId) throws StatisticsException {
    if(logger.isDebugEnabled()) {
      logger.debug("Remove request with id="+requestId);
    }
    SponsoredRequest r = storage.getRequest(requestId);
    if(r != null) {
      processor.cancel(requestId);
      storage.removeRequest(requestId);
      resultsManager.removeSponsoredResult(requestId);
    }
  }


  public void getSponsoredResult(int requestId, Visitor v) throws StatisticsException {
    resultsManager.getSponsoredResults(requestId, v);
  }

  public void cancelRequest(int requestId) throws StatisticsException {
    if(logger.isDebugEnabled()) {
      logger.debug("Cancel request with id="+requestId);
    }
    SponsoredRequest request = storage.getRequest(requestId);
    if(request == null) {
      throw new StatisticsException("Request is not found");
    }
    processor.cancel(requestId);
    storage.changeStatus(requestId, SponsoredRequest.Status.CANCELED);
  }

  public void shutdown() {
    if(processor != null) {
      processor.shutdown();
    }
  }

}
