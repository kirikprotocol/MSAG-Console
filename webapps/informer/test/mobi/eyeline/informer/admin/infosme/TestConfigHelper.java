package mobi.eyeline.informer.admin.infosme;

import mobi.eyeline.informer.admin.AdminException;

import java.util.HashSet;
import java.util.Set;

/**
 * @author Aleksandr Khalitov
 */
class TestConfigHelper {

  protected Set<String> obj = new HashSet<String>();

//  protected TestConfigHelper(Collection<String> obj) {
//    for(String s : obj) {
//      this.obj.add(s);
//    }
//  }

  synchronized void add(String obj) throws AdminException {
    if(obj.contains(obj)) {
      throw new InfosmeException("interaction_error","");
    }
    this.obj.add(obj);
  }

  synchronized void remove(String obj) throws AdminException {
    this.obj.remove(obj);
  }

  synchronized void update(String obj) throws AdminException {
  }
}
