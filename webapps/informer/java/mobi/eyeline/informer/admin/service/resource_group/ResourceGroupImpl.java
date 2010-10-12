package mobi.eyeline.informer.admin.service.resource_group;

import org.apache.log4j.Category;

abstract class ResourceGroupImpl implements ResourceGroup {

  protected String name;

  protected Category logger = Category.getInstance(this.getClass().getName());

  ResourceGroupImpl(String name) {
    this.name = name;
  }

  public String getName() {
    return name;
  }
}
