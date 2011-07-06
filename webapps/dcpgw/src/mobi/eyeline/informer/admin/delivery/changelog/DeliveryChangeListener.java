package mobi.eyeline.informer.admin.delivery.changelog;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Интерфейс подписчика DeliveryChangeDetector-а
 *
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.11.2010
 * Time: 18:43:59
 */
public interface DeliveryChangeListener {

  public void messageStateChanged(ChangeMessageStateEvent e) throws AdminException;

  public void deliveryStateChanged(ChangeDeliveryStatusEvent e) throws AdminException;

}
