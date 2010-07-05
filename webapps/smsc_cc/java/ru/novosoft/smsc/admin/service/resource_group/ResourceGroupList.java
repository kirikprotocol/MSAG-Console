package ru.novosoft.smsc.admin.service.resource_group;

import ru.novosoft.smsc.admin.AdminException;

import java.util.*;

/**
 * Список ресурс-групп
 */
class ResourceGroupList {
  
  private Map<String, ResourceGroup> resourceGroups = new HashMap<String, ResourceGroup>();

  public void add(ResourceGroup d)
      throws AdminException {
    if (resourceGroups.containsKey(d.getName()))
      throw new AdminException("Resource group \"" + d.getName() + "\" already contained in list");
    resourceGroups.put(d.getName(), d);
  }

  public ResourceGroup remove(String name) throws AdminException {
    require(name);
    return resourceGroups.remove(name);
  }

  private void require(String name) throws AdminException {
    if (!resourceGroups.containsKey(name))
      throw new AdminException("Unknown resource group \"" + name + "\"");
  }

  public List<String> getResourceGroupsNames() {
    return new ArrayList<String>(resourceGroups.keySet());
  }

  public ResourceGroup get(String name) throws AdminException {
    require(name);
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