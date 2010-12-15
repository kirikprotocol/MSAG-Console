package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.Delivery;

import java.lang.reflect.Method;
import java.text.SimpleDateFormat;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
class DeliveriesDiffHelper extends DiffHelper {

  DeliveriesDiffHelper() {
    super(Subject.DELIVERIES);
  }

  void logDeliveryCreated(Journal j, String user, int id, String deliveryName) throws AdminException {
    j.addRecord(JournalRecord.Type.ADD, subject, user, "delivery_created", Integer.toString(id), deliveryName);
  }

  void logDeliveryDroped(Journal j, String user, int id, String deliveryName) throws AdminException {
    j.addRecord(JournalRecord.Type.REMOVE, subject, user, "delivery_removed", Integer.toString(id), deliveryName);
  }

  void logDeliveryPaused(Journal j, String user, int id) throws AdminException {
    j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_paused", Integer.toString(id));
  }

  void logDeliveryActivated(Journal j, String user, int id) throws AdminException {
    j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_activate", Integer.toString(id));
  }

  void logChanges(Journal j, String user, Delivery oldDelivery, Delivery newDelivery) throws AdminException {
    List<Method> getters = getGetters(Delivery.class, "getProperty", "getProperties", "getActivePeriodEnd", "getActivePeriodStart", "getStartDate", "getEndDate", "getValidityDate", "getRetryPolicy");
    List<Object> oldValues = callGetters(getters, oldDelivery);
    List<Object> newValues = callGetters(getters, newDelivery);
    logChanges(j, oldValues, newValues, getters, user, "delivery_property_changed", newDelivery.getName());
    SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm");
    String temp1, temp2;
    if (oldDelivery.getActivePeriodEnd() != null && newDelivery.getActivePeriodEnd() != null &&
        !((temp1 = oldDelivery.getActivePeriodEnd().toString())).equals((temp2 = newDelivery.getActivePeriodEnd().toString()))) {

      j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_property_changed", "activePeriodEnd", temp1, temp2, oldDelivery.getName());

    }
    if (oldDelivery.getActivePeriodStart() != null && newDelivery.getActivePeriodStart() != null &&
        !(temp1 = oldDelivery.getActivePeriodStart().toString()).equals((temp2 = newDelivery.getActivePeriodStart().toString()))) {

      j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_property_changed", "activePeriodStart", temp1, temp2, oldDelivery.getName());

    }
    if (oldDelivery.getStartDate() != null && newDelivery.getStartDate() != null &&
        !(temp1 = dateFormat.format(oldDelivery.getStartDate())).equals((temp2 = dateFormat.format(newDelivery.getStartDate())))) {

      j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_property_changed", "startDate", temp1, temp2, oldDelivery.getName());

    }
    if (oldDelivery.getEndDate() != null && newDelivery.getEndDate() != null &&
        !(temp1 = dateFormat.format(oldDelivery.getEndDate())).equals((temp2 = dateFormat.format(newDelivery.getEndDate())))) {

      j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_property_changed", "endDate", temp1, temp2, oldDelivery.getName());
    }
    if (((temp1 = oldDelivery.getProperty(UserDataConsts.SMS_NOTIF_ADDRESS)) != null &&
        !temp1.equals(temp2 = newDelivery.getProperty(UserDataConsts.SMS_NOTIF_ADDRESS))) ||
        ((temp2 = newDelivery.getProperty(UserDataConsts.SMS_NOTIF_ADDRESS)) != null && !temp2.equals(temp1))) {

      j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_property_changed", "sms notification",
          temp1 == null ? "" : temp1, temp2 == null ? "" : temp2, oldDelivery.getName());
    }
    if (((temp1 = oldDelivery.getProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS)) != null &&
        !temp1.equals(temp2 = newDelivery.getProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS))) ||
        ((temp2 = newDelivery.getProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS)) != null && !temp2.equals(temp1))) {

      j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_property_changed", "email notification",
          temp1 == null ? "" : temp1, temp2 == null ? "" : temp2, oldDelivery.getName());
    }
    if (((temp1 = oldDelivery.getProperty(UserDataConsts.SECRET)) != null &&
        !temp1.equals(temp2 = newDelivery.getProperty(UserDataConsts.SECRET))) ||
        ((temp2 = newDelivery.getProperty(UserDataConsts.SECRET)) != null && !temp2.equals(temp1))) {

      j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_property_changed", "secret",
          temp1 == null ? "" : temp1, temp2 == null ? "" : temp2, oldDelivery.getName());
    }
    if (((temp1 = oldDelivery.getProperty(UserDataConsts.SECRET_TEXT)) != null &&
        !temp1.equals(temp2 = newDelivery.getProperty(UserDataConsts.SECRET_TEXT))) ||
        ((temp2 = newDelivery.getProperty(UserDataConsts.SECRET_TEXT)) != null && !temp2.equals(temp1))) {

      j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_property_changed", "secret text",
          temp1 == null ? "" : temp1, temp2 == null ? "" : temp2, oldDelivery.getName());
    }
    if (((temp1 = oldDelivery.getProperty(UserDataConsts.SECRET_FLASH)) != null &&
        !temp1.equals(temp2 = newDelivery.getProperty(UserDataConsts.SECRET_FLASH))) ||
        ((temp2 = newDelivery.getProperty(UserDataConsts.SECRET_FLASH)) != null && !temp2.equals(temp1))) {

      j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_property_changed", "secret flash",
          temp1 == null ? "" : temp1, temp2 == null ? "" : temp2, oldDelivery.getName());
    }

    temp1 = oldDelivery.getRetryPolicy();
    if("".equals(temp1)) {
      temp1 = "default";
    }
    temp2 = newDelivery.getRetryPolicy();
    if("".equals(temp2)) {
      temp2 = "default";
    }
    if((temp1 == null && temp2 != null) || (temp1 != null && temp2 == null) || temp1 != null) {
      j.addRecord(JournalRecord.Type.CHANGE, subject, user, "delivery_property_changed", "retryPolicy",
          temp1 == null ? "" : temp1, temp2 == null ? "" : temp2, oldDelivery.getName());
    }

  }


}
