package ru.novosoft.smsc.web.journal;

import ru.novosoft.smsc.admin.sme.Sme;

import java.lang.reflect.Method;
import java.util.List;

/**
 * @author Artem Snopkov
 */
class SmeDiffHelper extends DiffHelper {
  public SmeDiffHelper(String subject) {
    super(subject);
  }

  public void logChanges(Journal j, Sme oldSme, Sme newSme, String user) {
    List<Method> getters = getGetters(Sme.class);
    List<Object> oldValues = callGetters(getters, oldSme);
    List<Object> newValues = callGetters(getters, newSme);
    logChanges(j, oldValues, newValues, getters, user);
  }
}
