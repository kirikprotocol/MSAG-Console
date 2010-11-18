package mobi.eyeline.informer.admin.service.resource_group;

import org.apache.log4j.Category;

abstract class ResourceGroupImpl implements ResourceGroup {

  protected final String name;

  protected final Category logger = Category.getInstance(this.getClass().getName());

  ResourceGroupImpl(String name) {
    this.name = name;
  }

  public String getName() {
    return name;
  }
}
