package mobi.eyeline.util.jsf.components;

import java.util.Collection;
import java.util.Map;
import java.util.Set;

/**
 * @author Aleksandr Khalitov
 */
public class ResourceBean {

  public Map getUrl() {

    //Dummy map
    return new Map() {

      public int size() {
        return 0;
      }

      public boolean isEmpty() {
        return false;
      }

      public boolean containsKey(Object key) {
        return false;
      }

      public boolean containsValue(Object value) {
        return false;
      }

      public Object get(Object key) {
        if(key == null) {
          return null;
        }else {
          return ResourceUtils.getResourceUrl(key.toString());
        }
      }

      public Object put(Object key, Object value) {
        return null;
      }

      public Object remove(Object key) {
        return null;
      }

      public void putAll(Map m) {
      }

      public void clear() {
      }

      public Set keySet() {
        return null;
      }

      public Collection values() {
        return null;
      }


      public Set entrySet() {
        return null;
      }
    };

  }
}
