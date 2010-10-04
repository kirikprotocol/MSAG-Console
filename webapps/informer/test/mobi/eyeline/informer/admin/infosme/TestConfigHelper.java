package mobi.eyeline.informer.admin.infosme;

import mobi.eyeline.informer.admin.AdminException;

import java.util.HashSet;
import java.util.Set;

/**
 * @author Aleksandr Khalitov
 */
class TestConfigHelper {

  protected Set<String> objects = new HashSet<String>();

//  protected TestConfigHelper(Collection<String> obj) {
//    for(String s : obj) {
//      this.obj.add(s);
//    }
//  }

  synchronized void add(String o) throws AdminException {
    if(objects.contains(o)) {
      throw new InfosmeException("interaction_error","");
    }
    this.objects.add(o);
  }

  synchronized void remove(String o) throws AdminException {
    this.objects.remove(o);
  }

  synchronized void update(String o) throws AdminException {
  }
}
