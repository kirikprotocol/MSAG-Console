package ru.sibinco.sponsored.stats.beans;

import ru.sibinco.sponsored.stats.backend.StatRequestManager;
import ru.sibinco.sponsored.stats.backend.StatisticsException;

import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class SponsorRequestsTableHelper extends RequestsTableHelper{

  private final StatRequestManager requestManager;

  SponsorRequestsTableHelper(String uid, StatRequestManager requestManager, RequestFilter filter) {
    super(uid, filter);
    this.requestManager = requestManager;
  }

  protected List loadRequests() throws StatisticsException {
    return requestManager.getSponsoreRequests();
  }
}
