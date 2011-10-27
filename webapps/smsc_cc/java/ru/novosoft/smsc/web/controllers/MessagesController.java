package ru.novosoft.smsc.web.controllers;

import javax.faces.application.FacesMessage;
import javax.faces.context.FacesContext;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * User: artem
 * Date: 27.10.11
 */
public class MessagesController extends SmscController {

  public boolean isHasErrors() {
    return hasMessages("smsc_errors", FacesMessage.SEVERITY_ERROR);
  }

  public List<String> getErrors() {
    return getMessages("smsc_errors", FacesMessage.SEVERITY_ERROR);
  }

  public boolean isHasWarnings() {
    return hasMessages("smsc_errors", FacesMessage.SEVERITY_WARN);
  }

  public List<String> getWarnings() {
    return getMessages("smsc_errors", FacesMessage.SEVERITY_WARN);
  }

  public boolean hasMessages(String category, FacesMessage.Severity severity) {
    FacesContext fc = FacesContext.getCurrentInstance();
    for (Iterator<FacesMessage> iter = fc.getMessages(category); iter.hasNext();) {
      FacesMessage m = iter.next();
      if ((severity == null || m.getSeverity() == severity))
        return true;
    }
    return false;
  }

  public List<String> getMessages(String category, FacesMessage.Severity severity) {
    FacesContext fc = FacesContext.getCurrentInstance();
    List<String> result = new ArrayList<String>();
    for (Iterator<FacesMessage> iter = fc.getMessages(category); iter.hasNext();) {
      FacesMessage m = iter.next();
      if ((severity == null || m.getSeverity() == severity))
        result.add(m.getSummary());
    }
    return result;
  }

}
