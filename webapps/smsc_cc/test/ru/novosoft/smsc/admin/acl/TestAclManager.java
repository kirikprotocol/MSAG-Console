package ru.novosoft.smsc.admin.acl;

import ru.novosoft.smsc.admin.cluster_controller.ClusterController;

/**
 * @author Artem Snopkov
 */
public class TestAclManager extends AclManagerImpl {
  public TestAclManager(ClusterController cc) {
    super(cc);
  }
}
