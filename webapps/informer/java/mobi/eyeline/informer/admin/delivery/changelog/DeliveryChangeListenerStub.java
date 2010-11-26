package mobi.eyeline.informer.admin.delivery.changelog;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Пустая реализация DeliveryChangeListener-а
 *
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 23.11.2010
 * Time: 11:30:58
 */


public class DeliveryChangeListenerStub implements DeliveryChangeListener {

  public void messageStateChanged(ChangeMessageStateEvent e) throws AdminException {
  }

  public void deliveryStateChanged(ChangeDeliveryStatusEvent e) throws AdminException {
  }
}
