package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;

import java.lang.reflect.Method;
import java.text.SimpleDateFormat;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
class DeliveriesDiffHelper extends DiffHelper{

  DeliveriesDiffHelper() {
    super(Subject.DELIVERIES);
  }

  void logDeliveryPaused(Journal j, String user, int id) throws AdminException {
    j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_paused", Integer.toString(id));
  }

  void logDeliveryActivated(Journal j, String user, int id) throws AdminException {
    j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_activate", Integer.toString(id));
  }

  void logChanges(Journal j, String user, Delivery oldDelivery, Delivery newDelivery) throws AdminException {
    List<Method> getters = getGetters(Delivery.class, "getActivePeriodEnd","getActivePeriodStart", "getStartDate", "getEndDate", "getValidityDate");
    List<Object> oldValues = callGetters(getters, oldDelivery);
    List<Object> newValues = callGetters(getters, newDelivery);
    logChanges(j, oldValues, newValues, getters, user, "delivery_property_changed", newDelivery.getName());
    SimpleDateFormat timeFormat = new SimpleDateFormat("HH:mm");
    SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm");
    String temp1, temp2;
    if(oldDelivery.getActivePeriodEnd() != null && newDelivery.getActivePeriodEnd() != null &&
        !(temp1 = timeFormat.format(oldDelivery.getActivePeriodEnd())).equals((temp2 = timeFormat.format(newDelivery.getActivePeriodEnd())))) {

      j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_property_changed", "activePeriodEnd", temp1, temp2, oldDelivery.getName());

    }
    if(oldDelivery.getActivePeriodStart() != null && newDelivery.getActivePeriodStart() != null &&
        !(temp1 = timeFormat.format(oldDelivery.getActivePeriodStart())).equals((temp2 = timeFormat.format(newDelivery.getActivePeriodStart())))) {

      j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_property_changed", "activePeriodStart", temp1, temp2, oldDelivery.getName());

    }
    if(oldDelivery.getStartDate() != null && newDelivery.getStartDate() != null &&
        !(temp1 = dateFormat.format(oldDelivery.getStartDate())).equals((temp2 = dateFormat.format(newDelivery.getStartDate())))) {

      j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_property_changed", "startDate", temp1, temp2, oldDelivery.getName());

    }
    if(oldDelivery.getEndDate() != null && newDelivery.getEndDate() != null &&
        !(temp1 = dateFormat.format(oldDelivery.getEndDate())).equals((temp2 = dateFormat.format(newDelivery.getEndDate())))) {

      j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_property_changed", "endDate", temp1, temp2, oldDelivery.getName());
    }

    if(oldDelivery.getValidityDate() == null && newDelivery.getValidityDate() != null) {
      j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_property_changed", "validityDate", "", dateFormat.format(newDelivery.getValidityDate()), oldDelivery.getName());
    }else if(oldDelivery.getValidityDate() != null && newDelivery.getValidityDate() == null) {
      j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_property_changed", "validityDate", dateFormat.format(oldDelivery.getValidityDate()), "", oldDelivery.getName());
    }else if(newDelivery.getValidityDate() != null && oldDelivery.getValidityDate() != null && !(temp1 = dateFormat.format(oldDelivery.getValidityDate())).equals((temp2 = dateFormat.format(newDelivery.getValidityDate())))) {
      j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_property_changed", "validityDate", temp1, temp2, oldDelivery.getName());
    }
  }


}
