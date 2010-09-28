package mobi.eyeline.informer.admin.infosme;

import mobi.eyeline.informer.admin.AdminException;

import java.util.LinkedList;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class TestCategoriesHelper {

  private List<Category> categories = new LinkedList<Category>();

  public TestCategoriesHelper() {
    categories.add(new Category().setName("ru.novosoft.informer").setLevel("FATAL"));
    categories.add(new Category().setName("ru.novosoft.informer.admin").setLevel("INFO"));
  }

  public List<Category> getCategories()  throws AdminException {
    return categories;
  }

  public void setCategories(List<Category> categories) throws AdminException{
    this.categories = categories;         //todo validation?                                               
  }
}
