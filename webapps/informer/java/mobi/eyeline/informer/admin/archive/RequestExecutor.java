package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;

import java.util.Date;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * @author Aleksandr Khalitov
 */
class RequestExecutor {

  private final Lock lock = new ReentrantLock();

  private final ArchiveContext context;

  private final ResultsManager resultsManager;

  RequestExecutor(ArchiveContext context, ResultsManager resultsManager) throws AdminException{
    this.context = context;
    this.resultsManager = resultsManager;
  }

  private static DeliveryFilter createFilter(DeliveriesRequest request) {
    DeliveryFilter filter = new DeliveryFilter();
    filter.setEndDateTo(request.getTill());
    filter.setStartDateFrom(request.getFrom());
    if(request.getOwner() != null) {
      filter.setUserIdFilter(request.getOwner());
    }
    if(request.getDeliveryName() != null) {
      filter.setNameFilter(request.getDeliveryName());
    }
    return filter;
  }
  private static DeliveryFilter createFilter(MessagesRequest request) {
    DeliveryFilter filter = new DeliveryFilter();
    filter.setEndDateTo(request.getTill());
    filter.setStartDateFrom(request.getFrom());
    return filter;
  }
  private static MessageFilter createFilter(MessagesRequest request, int deliveryId, Date endDate) {
    MessageFilter filter = new MessageFilter(deliveryId, request.getStartDate(), endDate == null ? new Date() : endDate);
    filter.setMsisdnFilter(request.getAddress().getSimpleAddress());
    return filter;
  }


  void execute(final DeliveriesRequest request) throws AdminException{
    final User u = context.getUser(request.getCreater());
    try{
      lock.lock();
      GetDeliveriesStrategy strategy = getDeliveriesStrategy(request);
      strategy.getResults(u, request);
    }finally {
      lock.unlock();
    }
  }

  void execute(final MessagesRequest request) throws AdminException{
    final User u = context.getUser(request.getCreater());
    try{
      lock.lock();
      DeliveryFilter deliveryFilter = createFilter(request);

      final ResultsManager.MessagesResult[] messagesResults = new ResultsManager.MessagesResult[]{null};

      final UnmodifiableDeliveryManager dm = context.getDeliveryManager();


      try{
        messagesResults[0] = resultsManager.createMessagesResutls(request.getId());

        final int totalDeliveries = dm.countDeliveries(u.getLogin(), u.getPassword(), deliveryFilter);

        final float[] deliveryCounter = new float[]{0};

        dm.getDeliveries(u.getLogin(), u.getPassword(), deliveryFilter, 1000,
            new Visitor<Delivery>() {
              public boolean visit(final Delivery delivery) throws AdminException {

                final int deliveryId = delivery.getId();

                MessageFilter messageFilter = createFilter(request, deliveryId, delivery.getEndDate());

                dm.countMessages(u.getLogin(), u.getPassword(), messageFilter);

                dm.getMessages(u.getLogin(), u.getPassword(), messageFilter, 1000,
                    new Visitor<Message>() {
                      public boolean visit(Message message) throws AdminException {
                        messagesResults[0].write(delivery, message);
                        return true;
                      }
                    }
                );
                deliveryCounter[0]++;
                request.setProgress((int)(100*deliveryCounter[0]/totalDeliveries));
                return true;
              }
            });
        request.setProgress(100);
      }finally {
        if(messagesResults[0] != null) {
          messagesResults[0].close();
        }
      }
    }finally {
      lock.unlock();
    }
  }


  private GetDeliveriesStrategy getDeliveriesStrategy(DeliveriesRequest request) {
    return request.getDeliveryId() == null ? new MultiGetStrategy() : new SingleGetStrategy();
  }


  private static interface GetDeliveriesStrategy {

    void getResults(User u, DeliveriesRequest request) throws AdminException;
  }



  private class MultiGetStrategy implements GetDeliveriesStrategy {

    @Override
    public void getResults(final User u, final DeliveriesRequest request) throws AdminException {
      DeliveryFilter filter = createFilter(request);

      final ResultsManager.DeliveriesResult[] deliveriesResult = new ResultsManager.DeliveriesResult[]{null};

      final UnmodifiableDeliveryManager dm = context.getDeliveryManager();

      try{
        deliveriesResult[0] = resultsManager.createDeliveriesResutls(request.getId());

        final int[] total = new int[] {dm.countDeliveries(u.getLogin(), u.getPassword(), filter)};

        dm.getDeliveries(u.getLogin(), u.getPassword(), filter, 1000, new Visitor<Delivery>() {
          float counter = 0;

          public boolean visit(Delivery value) throws AdminException {
            final DeliveryStatistics stats = dm.getDeliveryStats(u.getLogin(), u.getPassword(), value.getId());
            final DeliveryStatusHistory history = dm.getDeliveryStatusHistory(u.getLogin(), u.getPassword(), value.getId());
            deliveriesResult[0].write(value, stats, history);
            request.setProgress((int) (100 * counter / total[0]));
            counter++;
            return true;
          }
        });
        request.setProgress(100);
      }finally {
        if(deliveriesResult[0] != null) {
          deliveriesResult[0].close();
        }
      }
    }
  }

  private class SingleGetStrategy implements GetDeliveriesStrategy {

    @Override
    public void getResults(final User u, final DeliveriesRequest request) throws AdminException {

      final ResultsManager.DeliveriesResult[] deliveriesResult = new ResultsManager.DeliveriesResult[]{null};

      final UnmodifiableDeliveryManager dm = context.getDeliveryManager();

      try{
        deliveriesResult[0] = resultsManager.createDeliveriesResutls(request.getId());

        final Delivery d = context.getDeliveryManager().getDelivery(u.getLogin(), u.getPassword(), request.getDeliveryId());
        final DeliveryStatistics stats = dm.getDeliveryStats(u.getLogin(), u.getPassword(), d.getId());
        final DeliveryStatusHistory history = dm.getDeliveryStatusHistory(u.getLogin(), u.getPassword(), d.getId());
        deliveriesResult[0].write(d, stats, history);

        request.setProgress(100);
      }finally {
        if(deliveriesResult[0] != null) {
          deliveriesResult[0].close();
        }
      }
    }
  }

}
