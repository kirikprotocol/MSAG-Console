package mobi.eyeline.informer.admin.archive;

import com.eyeline.utils.ThreadFactoryWithCounter;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import org.apache.log4j.Logger;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.*;

/**
 * @author Aleksandr Khalitov
 */

class RequestProcessor {

  private static final Logger logger = Logger.getLogger(RequestProcessor.class);

  private final RequestExecutor[] executors;

  private final ThreadPoolExecutor poolExecutor;

  private Map<Integer, SubmitedRequest> submited = new HashMap<Integer, SubmitedRequest>();

  private final RequestStorage requestStorage;

  RequestProcessor(ArchiveContext context, RequestStorage requestStorage, ResultsManager resultsManage, int requestChunkSize, int requestExecSize) throws InitException{
    this.requestStorage = requestStorage;
    executors = new RequestExecutor[requestChunkSize];
    try {
      for(int i=0; i<requestChunkSize; i++) {
        executors[i] = new RequestExecutor(context, resultsManage);
      }
    } catch (AdminException e) {
      throw new InitException(e);
    }
    poolExecutor = new ThreadPoolExecutor(2, requestExecSize, 60, TimeUnit.SECONDS, new LinkedBlockingQueue<Runnable>(),
        new ThreadFactoryWithCounter("ARCHIVE-REQUEST-EXEC-"));
  }

  private RequestExecutor getHandler(String creater) {
    return executors[Math.abs(creater.hashCode()% executors.length)];
  }

  void execute(final Request r) {
    final RequestExecutor executor = getHandler(r.getCreater());
    final CountDownLatch downLatch = new CountDownLatch(1);
    final Future future = poolExecutor.submit(new Runnable() {
      @Override
      public void run() {
        try {
          if(logger.isDebugEnabled()) {
            logger.debug("Execute request: "+r);
          }
          r.execute(executor);
          requestStorage.changeStatus(r.getId(), Request.Status.FINISHED);
        } catch (Exception e) {
          logger.error(e, e);
        } finally {
          try {
            downLatch.await();
          } catch (InterruptedException ignored) {}
          submited.remove(r.getId());
        }
      }
    });
    submited.put(r.getId(), new SubmitedRequest(future,r));
    downLatch.countDown();
  }

  void cancel(int requestId) {
    SubmitedRequest r = submited.remove(requestId);
    if(r != null) {
      r.cancel();
    }
  }

  Integer getProgress(int requestId) {
    SubmitedRequest r = submited.get(requestId);
    return r == null ? null : r.getCompleted();
  }

  void shutdown() {
    if(poolExecutor != null) {
      try{
        poolExecutor.shutdownNow();
      }catch (Exception ignored){}
    }
  }

  private class SubmitedRequest {

    private Future future;

    private Request request;

    private SubmitedRequest(Future future, Request request) {
      this.future = future;
      this.request = request;
    }

    private void cancel() {
      future.cancel(true);
    }

    private int getCompleted() {
      return request.getProgress();
    }
  }


}
