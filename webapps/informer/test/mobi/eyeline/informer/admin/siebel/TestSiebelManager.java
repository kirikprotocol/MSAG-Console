package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;
/**
 * @author Aleksandr Khalitov
 */
class TestSiebelManager extends SiebelManager{

  TestSiebelManager(TestSiebelDataProvider pv, SiebelContext context) throws AdminException {
    super(pv, context);
  }

  TestSiebelManager(SiebelContext context) throws AdminException {
    super(new TestSiebelDataProvider(), context);
  }


}
