package mobi.eyeline.informer.admin.delivery.changelog;

/**
 * @author Artem Snopkov
 */
public interface DeliveryChangesDetector {
  void addListener(DeliveryChangeListener listener);

  void removeListener(DeliveryChangeListener listener);

  void removeAllListeners();
}
