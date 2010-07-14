package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class TestClusterController extends ClusterController {

  public boolean applyRescheduleCalled;
  public boolean applyFraudCalled;
  public boolean applyMapLimitsCalled;

  public void applyReschedule() throws AdminException {
    applyRescheduleCalled = true;
  }


  public void applyFraud() throws AdminException {
    applyFraudCalled = true;
  }


  public void applyMapLimits() throws AdminException {
    applyMapLimitsCalled = true;
  }
}
