package ru.novosoft.smsc.jsp.smsc.categories;

import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.jsp.util.tables.impl.category.CategoryFilter;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

/**
 * Created by andrey Date: 07.02.2005 Time: 12:45:18
 */
public class CategoriesFilter extends SmscBean
{
  private CategoryFilter filter = null;

  private String nameFilter = null;
  private String initialized = null;

  private String mbApply = null;
  private String mbClear = null;
  private String mbCancel = null;


  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    filter = preferences.getCategoryFilter();

    if (initialized == null) {
      nameFilter = filter.getNameFilter();
    }
    if (nameFilter == null)
      nameFilter = "";

    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbApply != null) {
      filter.setNameFilter(nameFilter);
      return RESULT_DONE;
    }
    else if (mbClear != null) {
      nameFilter = "";
      return RESULT_OK;
    }
    else if (mbCancel != null)
      return RESULT_DONE;

    return RESULT_OK;
  }

  public CategoryFilter getFilter()
  {
    return filter;
  }

  public void setFilter(CategoryFilter filter)
  {
    this.filter = filter;
  }

  public String getNameFilter()
  {
    return nameFilter;
  }

  public void setNameFilter(String nameFilter)
  {
    this.nameFilter = nameFilter;
  }

  public String getMbApply()
  {
    return mbApply;
  }

  public void setMbApply(String mbApply)
  {
    this.mbApply = mbApply;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public String getInitialized()
  {
    return initialized;
  }

  public void setInitialized(String initialized)
  {
    this.initialized = initialized;
  }

  public String getMbClear()
  {
    return mbClear;
  }

  public void setMbClear(String mbClear)
  {
    this.mbClear = mbClear;
  }
}
