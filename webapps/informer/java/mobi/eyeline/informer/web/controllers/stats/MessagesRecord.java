package mobi.eyeline.informer.web.controllers.stats;

/**
 * @author Aleksandr Khalitov
 */
public interface MessagesRecord{

  public long getNewMessages();

  public long getProcessMessages();

  public long getDeliveredMessages();

  public long getFailedMessages();

  public long getExpiredMessages();

  public long getDeliveredMessagesSMS();

  public long getFailedMessagesSMS();

  public long getExpiredMessagesSMS();

}
