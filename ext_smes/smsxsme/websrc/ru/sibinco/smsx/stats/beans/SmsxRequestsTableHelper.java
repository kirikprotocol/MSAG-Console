package ru.sibinco.smsx.stats.beans;

import ru.sibinco.smsx.stats.backend.StatRequestManager;
import ru.sibinco.smsx.stats.backend.StatisticsException;

import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class SmsxRequestsTableHelper extends RequestsTableHelper {

  private final StatRequestManager requestManager;

  SmsxRequestsTableHelper(String uid, RequestFilter filter, StatRequestManager requestManager) {
    super(uid, filter);
    this.requestManager = requestManager;
  }

  protected List loadRequests() throws StatisticsException {
    return requestManager.getSmsxRequests();
  }

}
