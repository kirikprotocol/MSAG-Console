package ru.novosoft.smsc.jsp.smsc.categories;

/**
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.impl.category.CategoryQuery;

import javax.servlet.http.HttpServletRequest;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.Iterator;

public class IndexCategory extends IndexBean
{
  public static final int RESULT_ADD = IndexBean.PRIVATE_RESULT;
  public static final int RESULT_EDIT = IndexBean.PRIVATE_RESULT + 1;
  public static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 2;
  private long id = -1;
  private String name = null;
  protected String editName = null;
  protected String editId = null;
  //private boolean add = false;
  protected QueryResultSet categories = null;
  protected CategoryManager categoryManager = null;
  protected String mbAdd = null;
  protected String mbDelete = null;
  protected String mbEdit = null;
  protected String[] checkedCategoryNames = new String[0];
  protected Set checkedCategoryNamesSet = new HashSet();

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;
    pageSize = preferences.getCategoriesPageSize();
    if (sort != null)
      preferences.setCategoriesSortOrder(sort);
    else
      sort = preferences.getCategoriesSortOrder();

    categoryManager = appContext.getCategoryManager();
    return RESULT_OK;
  }

  public int process(final HttpServletRequest request)
  {
    super.process(request);
    categories = new EmptyResultSet();

    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbAdd != null)
      return RESULT_ADD;
    else if (mbEdit != null)
      return RESULT_EDIT;
    else if (mbDelete != null) {
      int dresult = deleteCategories();
      if (dresult != RESULT_OK)
        return result;
      else
        return RESULT_DONE;
    }

    logger.debug("Providers.Index - process with sorting [" + preferences.getCategoriesSortOrder() + "]");
    CategoryQuery query = new CategoryQuery(pageSize, preferences.getCategoryFilter(), preferences.getCategoriesSortOrder(), startPosition);
    categories = appContext.getCategoryManager().query(query);
      if(request.getSession().getAttribute("CATEGORY_NAME") != null){
          categories = getCategoriesByName((String) request.getSession().getAttribute("CATEGORY_NAME"));
          request.getSession().removeAttribute("CATEGORY_NAME");
      }
    totalSize = categories.getTotalSize();

    checkedCategoryNamesSet.addAll(Arrays.asList(checkedCategoryNames));

    return RESULT_OK;
  }

    private QueryResultSet getCategoriesByName(String name) {
        boolean found = false;
        QueryResultSet categories = null;
        while (!found) {
            CategoryQuery query = new CategoryQuery(pageSize, preferences.getCategoryFilter(), preferences.getCategoriesSortOrder(), startPosition);
            categories = appContext.getCategoryManager().query(query);
            for (Iterator i = categories.iterator(); i.hasNext();) {
                DataItem item = (DataItem) i.next();
                String al = (String) item.getValue("name");
                if (al.equals(name)) {
                    found = true;
                }
            }
            if (!found) {
                startPosition += pageSize;
            }
        }
        return categories;
    }

  private int deleteCategories()
  {
    for (int i = 0; i < checkedCategoryNames.length; i++) {
      String checkedCategoryName = checkedCategoryNames[i];
      categoryManager.removeCategory(checkedCategoryName);
      journalAppend(SubjectTypes.TYPE_category, checkedCategoryName, Actions.ACTION_DEL);
    }
    setCheckedCategoryNames(new String[0]);
    appContext.getStatuses().setCategoriesChanged(true);
    return RESULT_OK;
  }

  /**
   * ***************************** properties *************************************
   */

  public boolean isCategoryChecked(String name)
  {
    return checkedCategoryNamesSet.contains(name);
  }

  public String[] getCheckedCategoryNames()
  {
    return checkedCategoryNames;
  }

  public void setCheckedCategoryNames(String[] checkedCategoryNames)
  {
    this.checkedCategoryNames = checkedCategoryNames;
  }

  public String getEditName()
  {
    return editName;
  }

  public void setEditName(String editName)
  {
    this.editName = editName;
  }

  public String getEditId()
  {
    return editId;
  }

  public void setEditId(String editId)
  {
    this.editId = editId;
  }

  public String getId()
  {
    return -1 == id ? null : String.valueOf(id);
  }

  public void setId(final String id)
  {
    this.id = Long.decode(id).longValue();
  }

  public String getName()
  {
    return name;
  }

  public void setName(final String name)
  {
    this.name = name;
  }


  public QueryResultSet getCategories()
  {
    return categories;
  }


  public String getMbAdd()
  {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd)
  {
    this.mbAdd = mbAdd;
  }

  public String getMbDelete()
  {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete)
  {
    this.mbDelete = mbDelete;
  }

  public String getMbEdit()
  {
    return mbEdit;
  }

  public void setMbEdit(String mbEdit)
  {
    this.mbEdit = mbEdit;
  }

  public CategoryManager getCategoryManager()
  {
    return categoryManager;
  }

  public void setCategoryManager(CategoryManager categoryManager)
  {
    this.categoryManager = categoryManager;
  }

  public Set getCheckedCategoryNamesSet()
  {
    return checkedCategoryNamesSet;
  }

  public void setCheckedCategoryNamesSet(Set checkedCategoryNamesSet)
  {
    this.checkedCategoryNamesSet = checkedCategoryNamesSet;
  }

  public void setCategories(QueryResultSet categories)
  {
    this.categories = categories;
  }
}
