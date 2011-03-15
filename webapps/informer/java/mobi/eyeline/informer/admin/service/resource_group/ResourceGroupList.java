package mobi.eyeline.informer.admin.service.resource_group;

import mobi.eyeline.informer.admin.AdminException;

import java.util.*;

/**
 * Список ресурс-групп
 */
class ResourceGroupList {

  private final Map<String, ResourceGroup> resourceGroups = new HashMap<String, ResourceGroup>();

  public void add(ResourceGroup d) {
    if (!resourceGroups.containsKey(d.getName()))
      resourceGroups.put(d.getName(), d);
  }

  public ResourceGroup remove(String name) throws AdminException {
    require(name);
    return resourceGroups.remove(name);
  }

  private void require(String name) throws AdminException {
    if (!resourceGroups.containsKey(name))
      throw new ResourceGroupException("unknown_resource_group", name);
  }

  public List<String> getResourceGroupsNames() {
    return new ArrayList<String>(resourceGroups.keySet());
  }

  public ResourceGroup get(String name) throws AdminException {
    return resourceGroups.get(name);
  }

  public boolean contains(String name) {
    return resourceGroups.containsKey(name);
  }

  public Iterator iterator() {
    return resourceGroups.values().iterator();
  }

  public void clear() {
    resourceGroups.clear();
  }
}
