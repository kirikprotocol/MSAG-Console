package ru.novosoft.smsc.infosme.backend;

import ru.novosoft.smsc.infosme.backend.config.tasks.Task;

import java.util.*;

/**
 * User: artem
 * Date: 13.05.2008
 */

public class MultiTask {
  private final Map tasks = new HashMap(20);

  public void addTask(int regionId, Task task) {
    tasks.put(new Integer(regionId), task);
  }

  public Task getTask(int regionId) {
    return (Task)tasks.get(new Integer(regionId));
  }

  public Collection tasks() {
    return tasks.values();
  }

  public boolean contains(String subject) {
    return tasks.containsKey(subject);
  }

  public Collection getRegionIds() {
    return new ArrayList(tasks.keySet());
  }

  public void setName(String name) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setName(name);
  }

  public void setAddress(String address) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setAddress(address);
  }

  public void setProvider(String provider) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setProvider(provider);
  }

  public void setEnabled(boolean enabled) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setEnabled(enabled);
  }

  public void setPriority(int priority) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setPriority(priority);
  }

  public void setRetryOnFail(boolean retryOnFail) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setRetryOnFail(retryOnFail);
  }

  public void setReplaceMessage(boolean replaceMessage) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setReplaceMessage(replaceMessage);
  }

  public void setSvcType(String svcType) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setSvcType(svcType);
  }

  public void setEndDate(Date endDate) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setEndDate(endDate);
  }

  public void setStartDate(Date startDate) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setStartDate(startDate);
  }

//  public void setRetryTime(String retryTime) {
//    for (Iterator iter = tasks().iterator(); iter.hasNext();)
//      ((Task)iter.next()).setRetryTime(retryTime);
//  }

  public void setValidityPeriod(Integer validityPeriod) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setValidityPeriod(validityPeriod);
  }

  public void setValidityDate(Date validityDate) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setValidityDate(validityDate);
  }

  public void setActivePeriodStart(Date activePeriodStart) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setActivePeriodStart(activePeriodStart);
  }

  public void setActivePeriodEnd(Date activePeriodEnd) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setActivePeriodEnd(activePeriodEnd);
  }

  public void setActiveWeekDays(Collection activeWeekDays) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setActiveWeekDays(activeWeekDays);
  }

  public void setQuery(String query) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setQuery(query);
  }

  public void setTemplate(String template) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setTemplate(template);
  }

  public void setText(String text) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setText(text);
  }

  public void setDsTimeout(int dsTimeout) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setDsTimeout(dsTimeout);
  }

  public void setMessagesCacheSize(int messagesCacheSize) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setMessagesCacheSize(messagesCacheSize);
  }

  public void setMessagesCacheSleep(int messagesCacheSleep) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setMessagesCacheSleep(messagesCacheSleep);
  }

  public void setTransactionMode(boolean transactionMode) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setTransactionMode(transactionMode);
  }

  public void setKeepHistory(boolean keepHistory) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setKeepHistory(keepHistory);
  }

  public void setUncommitedInGeneration(int uncommitedInGeneration) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setUncommitedInGeneration(uncommitedInGeneration);
  }

  public void setUncommitedInProcess(int uncommitedInProcess) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setUncommitedInProcess(uncommitedInProcess);
  }

  public void setTrackIntegrity(boolean trackIntegrity) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setTrackIntegrity(trackIntegrity);
  }

  public void setDelivery(boolean delivery) {
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      ((Task)iter.next()).setDelivery(delivery);
  }

  public int getActualRecordsSize() {
    int result = 0;
    for (Iterator iter = tasks().iterator(); iter.hasNext();)
      result += ((Task)iter.next()).getActualRecordsSize();
    return result;
  }

  public Iterator iterator() {
    return tasks.values().iterator();
  }

}
