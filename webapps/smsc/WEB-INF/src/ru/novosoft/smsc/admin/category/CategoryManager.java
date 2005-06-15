package ru.novosoft.smsc.admin.category;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.QueryResultSetImpl;
import ru.novosoft.smsc.jsp.util.tables.impl.category.CategoryDataItem;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;

import java.io.IOException;
import java.util.*;


/**
 * Created by andrey Date: 07.02.2005 Time: 12:45:18
 */
public class CategoryManager
{
  private final Map categories = Collections.synchronizedMap(new TreeMap());
  private int lastUsedCatId;
  private static final String SECTION_NAME_categories = "categories";
  private static final String PARAM_NAME_last_used_categories_id = "last used category id";
  protected SMSCAppContext appContext = null;
  private Config webappConfig = null;


  public CategoryManager(final Config gwConfig) throws Config.WrongParamTypeException, Config.ParamNotFoundException, NumberFormatException
  {
    this.webappConfig = gwConfig;
    if (!gwConfig.containsSection(SECTION_NAME_categories)) {
      gwConfig.setInt(SECTION_NAME_categories + '.' + PARAM_NAME_last_used_categories_id, 0);
    }
    lastUsedCatId = gwConfig.getInt(SECTION_NAME_categories + '.' + PARAM_NAME_last_used_categories_id);
    final Collection categoryIds = gwConfig.getSectionChildShortParamsNames(SECTION_NAME_categories);
    for (Iterator i = categoryIds.iterator(); i.hasNext();) {
      final String categoryIdStr = (String) i.next();
      if (!PARAM_NAME_last_used_categories_id.equalsIgnoreCase(categoryIdStr)) {
        final Long categoryId = Long.decode(categoryIdStr);
        final Category category = createCategory(categoryId.longValue(), gwConfig.getString(SECTION_NAME_categories + '.' + categoryIdStr));
        categories.put(categoryId, category);
      }
    }
  }

  public synchronized QueryResultSet query(Query query_to_run)
  {
    Vector sortOrder = query_to_run.getSortOrder();
    boolean isNegativeSort = false;
    String sort = (String) sortOrder.get(0);

    if (sort.charAt(0) == '-') {
      sort = sort.substring(1);
      isNegativeSort = true;
    }

    QueryResultSetImpl result = new QueryResultSetImpl(Category.columnNames, query_to_run.getSortOrder());

    List filteredCategories = filterCategories(query_to_run.getFilter());
    sortCategories(sort, isNegativeSort, filteredCategories);
    result.setLast(true);
    int lastpos = Math.min(query_to_run.getStartPosition() + query_to_run.getExpectedResultsQuantity(), filteredCategories.size());
    for (int i = query_to_run.getStartPosition(); i < lastpos; i++) {
      result.add((CategoryDataItem) filteredCategories.get(i));
      result.setLast(i == (filteredCategories.size() - 1));
    }
    result.setTotalSize(filteredCategories.size());

    return result;
  }

  private List filterCategories(Filter filter)
  {
    List result = new LinkedList();
    for (Iterator i = categories.values().iterator(); i.hasNext();) {
      Category category = (Category) i.next();
      CategoryDataItem categoryItem = new CategoryDataItem(category);
      if (filter.isItemAllowed(categoryItem))
        result.add(categoryItem);
    }
    return result;
  }

  private void sortCategories(final String sort, final boolean isNegativeSort, List filteredCategories)
  {
    Collections.sort(filteredCategories, new Comparator()
    {
      public int compare(Object o1, Object o2)
      {
        if (o1 instanceof CategoryDataItem && o2 instanceof CategoryDataItem) {
          CategoryDataItem u1 = (CategoryDataItem) o1;
          CategoryDataItem u2 = (CategoryDataItem) o2;
          String ut1 = (String) u1.getValue(sort);
          String ut2 = (String) u2.getValue(sort);
          int result = ut1.compareToIgnoreCase(ut2);
          //int result = ((String) u1.getValue(sort)).compareToIgnoreCase((String) u2.getValue(sort));
          return isNegativeSort
                  ? -result
                  : result;
        }
        else
          return 0;
      }
    });
  }

  public synchronized Map getCategories()
  {
    return categories;
  }

  public synchronized Category getCategory(Long id)
  {  //final Long categoryId = Long.decode(id);
    return (Category) categories.get(id);
  }

  public synchronized Category getCategoryByName(String name)
  {
    Category categoryFounded = null;
    for (Iterator iterator = categories.values().iterator(); iterator.hasNext();) {
      Category category = (Category) iterator.next();
      if (category.getName().equals(name)) {
        categoryFounded = category;
      }
    }
    return categoryFounded;
  }

  private synchronized Category createCategory(final long id, final String name)
  {
    final Category category = new Category(id, name);
    categories.put(new Long(category.getId()), category);
    return category;
  }

  public synchronized Category createCategory(final String name)
  {
    return createCategory(++lastUsedCatId, name);
  }

  public synchronized void setCategoryName(final long id, final String name) throws NullPointerException
  {
    final Category category = (Category) categories.get(new Long(id));
    if (null == category)
      throw new NullPointerException("Category \"" + id + "\" not found.");
    category.setName(name);
  }

  public synchronized void apply()
          throws Exception
  {// webappConfig= appContext.getConfig();
    store(webappConfig);
    // XmlAuthenticator.init(configFile);
  }

  public synchronized void store(final Config gwConfig) throws Config.WrongParamTypeException, IOException
  {
    gwConfig.removeSection(SECTION_NAME_categories);
    gwConfig.setInt(SECTION_NAME_categories + '.' + PARAM_NAME_last_used_categories_id, lastUsedCatId);
    for (Iterator i = categories.values().iterator(); i.hasNext();) {
      final Category category = (Category) i.next();
      gwConfig.setString(SECTION_NAME_categories + "." + category.getId(), StringEncoderDecoder.encode(category.getName()));
    }
    gwConfig.save();
  }

  public Category removeCategory(String name)
  {
  //  Category categoryFounded = null;
    long id = 0;
    for (Iterator i = categories.values().iterator(); i.hasNext();) {
      Category category = (Category) i.next();
      if (category.getName().equals(name)) {
        id = category.getId();
      }
    }
    final Long categoryId = new Long(id);
    return (Category) categories.remove(categoryId);
  }
}
