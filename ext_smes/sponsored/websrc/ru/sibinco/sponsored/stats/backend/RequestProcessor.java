package ru.sibinco.sponsored.stats.backend;

import org.apache.log4j.Category;
import ru.sibinco.sponsored.stats.backend.datasource.FileDeliveryStatDataSource;

import java.io.File;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
class RequestProcessor {

  private static final Category logger = Category.getInstance(RequestProcessor.class);

  private final RequestExecutor requestExecutor;

  private final Map submited = new HashMap();

  private final RequestStorage requestStorage;

  private final Object lock = new Object();

  RequestProcessor(RequestStorage requestStorage, File sponsoredArtefacts,
                   ResultsManager resultsManager) throws StatisticsException {
    this.requestStorage = requestStorage;
    this.requestExecutor = new RequestExecutor(
        new FileDeliveryStatDataSource(sponsoredArtefacts),
        resultsManager);
  }

  void execute(final SponsoredRequest request) {
    final Integer reqId = new Integer(request.getId());
    final Object t_lock = new Object();
    Thread t = new Thread() {
      public void run() {
        synchronized (t_lock) {
          if(logger.isDebugEnabled()) {
            logger.debug("Start to process request: "+request);
          }
          try{
            request.execute(requestExecutor);
            requestStorage.changeStatus(request.getId(), SponsoredRequest.Status.READY);
            if(logger.isDebugEnabled()) {
              logger.debug("Request is processed: "+request);
            }
          }catch (StatisticsException e){
            logger.error(e, e);
            if(e.getCode() == StatisticsException.Code.COMMON) {
              try{
                requestStorage.changeStatus(request.getId(), SponsoredRequest.Status.ERROR);
                requestStorage.setError(request.getId(), e.getMessage());
              }catch (Exception ignored){}
            }

          }catch (Exception e){
            e.printStackTrace();
            logger.error(e, e);
            try{
              requestStorage.changeStatus(request.getId(), SponsoredRequest.Status.ERROR);
              requestStorage.setError(request.getId(), e.getMessage());
            }catch (Exception ignored){}
          }finally {
            synchronized (lock) {
              submited.remove(reqId);
            }
          }
        }
      }
    };
    synchronized (lock) {
      submited.put(reqId, new SubmitedRequest(t, request, t_lock));
    }
    t.start();
  }


  void cancel(int requestId) {
    SubmitedRequest t;
    synchronized (lock) {
      t = (SubmitedRequest)submited.get(new Integer(requestId));
    }
    if(t != null) {
      t.cancel();
      synchronized (t.lock){}
    }
  }

  void shutdown() {
    Iterator i = submited.values().iterator();
    while(i.hasNext()) {
      SubmitedRequest t = (SubmitedRequest)i.next();
      t.cancel();
    }
  }


  Integer getProgress(int requestId) {
    SubmitedRequest r = (SubmitedRequest)submited.get(new Integer(requestId));
    return r == null ? null : new Integer(r.request.getProgress());
  }


  private class SubmitedRequest {

    private final Thread thread;

    private final SponsoredRequest request;

    final private Object lock;

    private SubmitedRequest(Thread thread, SponsoredRequest request, Object lock) {
      this.thread = thread;
      this.request = request;
      this.lock = lock;
    }

    private void cancel() {
      thread.interrupt();
      synchronized (lock){}
    }

    private int getCompleted() {
      return request.getProgress();
    }
  }

}
