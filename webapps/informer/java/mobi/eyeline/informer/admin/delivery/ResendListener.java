package mobi.eyeline.informer.admin.delivery;

/**
 * author: Aleksandr Khalitov
 */
public interface ResendListener {

  public void resended(long messageId, int totalSize);

}
