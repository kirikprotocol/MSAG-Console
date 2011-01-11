package mobi.eyeline.informer.admin.delivery.changelog;

import org.apache.log4j.Logger;

import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public abstract class AbstractDeliveryChangesDetector implements DeliveryChangesDetector {

  Logger log = Logger.getLogger(this.getClass());

  protected final List<DeliveryChangeListener> listeners = Collections.synchronizedList(new LinkedList<DeliveryChangeListener>());

  @Override
  public void addListener(DeliveryChangeListener listener) {
    listeners.add(listener);
  }

  @Override
  public void removeListener(DeliveryChangeListener listener) {
    listeners.remove(listener);
  }

  @Override
  public void removeAllListeners() {
    listeners.clear();
  }

  protected void fireEvent(ChangeMessageStateEvent stateEvent) {
    for (DeliveryChangeListener listener : listeners) {
      try {
        listener.messageStateChanged(stateEvent);
      } catch (Exception e) {
        log.error("error in listener", e);
      }
    }
  }


  protected void fireEvent(ChangeDeliveryStatusEvent stateEventChange) {
    for (DeliveryChangeListener listener : listeners) {
      try {
        listener.deliveryStateChanged(stateEventChange);
      } catch (Exception e) {
        log.error("error in listener", e);
      }
    }
  }
}
