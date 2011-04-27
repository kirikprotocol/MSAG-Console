package ru.sibinco.smsx.stats.backend;

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

  public StatRequestManager(File requestsDir, File smsxArtefacts) throws StatisticsException {
    this.storage = new RequestStorage(requestsDir);
    this.resultsManager = new ResultsManager(requestsDir);
    this.processor = new RequestProcessor(storage, smsxArtefacts, new ResultsManager(requestsDir));

    Iterator i = storage.getSmsxRequests().iterator();
    while(i.hasNext()) {
      SmsxRequest r = (SmsxRequest)i.next();
      if (r.getId() > lastId) {
        lastId = r.getId();
      }
      if(r.getStatus() == SmsxRequest.Status.IN_PROCESS) {
        processor.execute(r);
      }
    }
  }

  public SmsxRequest createRequest(SmsxRequestPrototype request) throws StatisticsException {
    SmsxRequest smsxRequest = new SmsxRequest();
    smsxRequest.copyFrom(request);
    smsxRequest.validate();
    smsxRequest.setId(getNextId());

    if(logger.isDebugEnabled()) {
      logger.debug("Create request: "+request);
    }

    storage.createRequest(smsxRequest);

    processor.execute(smsxRequest);

    return smsxRequest.copy();
  }


  public SmsxRequest getRequest(int requestId) throws StatisticsException {
    SmsxRequest q = storage.getRequest(requestId);
    if(q == null) {
      return null;
    }
    setStatus(q);
    return q;
  }

  private void setStatus(SmsxRequest q) {
    if(q.getStatus() != SmsxRequest.Status.CANCELED && q.getStatus() != SmsxRequest.Status.READY && q.getStatus() != SmsxRequest.Status.ERROR) {
      q.setStatus(SmsxRequest.Status.IN_PROCESS);
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

  public List getSmsxRequests() throws StatisticsException {
    List c = storage.getSmsxRequests();
    Iterator i = c.iterator();
    while(i.hasNext()) {
      setStatus((SmsxRequest)i.next());
    }
    return c;
  }

  public void removeRequest(int requestId) throws StatisticsException {
    if(logger.isDebugEnabled()) {
      logger.debug("Remove request with id="+requestId);
    }
    SmsxRequest r = storage.getRequest(requestId);
    if(r != null) {
      processor.cancel(requestId);
      storage.removeRequest(requestId);
      resultsManager.removeSmsxResult(requestId);
    }
  }

  public SmsxResults getSmsxResult(int requestId) {
    return resultsManager.getSmsxResult(requestId);
  }

  public void cancelRequest(int requestId) throws StatisticsException {
    if(logger.isDebugEnabled()) {
      logger.debug("Cancel request with id="+requestId);
    }
    SmsxRequest request = storage.getRequest(requestId);
    if(request == null) {
      throw new StatisticsException("Request is not found");
    }
    processor.cancel(requestId);
    storage.changeStatus(requestId, SmsxRequest.Status.CANCELED);
  }

  public void shutdown() {
    if(processor != null) {
      processor.shutdown();
    }
  }

}
