package ru.novosoft.smsc.jsp.util.tables.impl.smcstat;

/**
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.category.Category;

public class StatRouteFilter implements Filter
{
  private Long providerId = null;
  private Long categoryId = null;
  private static final int ALLOWED = 0;
  private static final int NOT_ALLOWED = 1;
  private static final int UNKNOWN = 2;


  public boolean isEmpty()
  {
    return this.providerId == null && this.categoryId == null;
  }

  public Long getProviderId()
  {
    return providerId;
  }

  public void setProviderId(Long providerId)
  {
    this.providerId = providerId;
  }

  public Long getCategoryId()
  {
    return categoryId;
  }

  public void setCategoryId(Long categoryId)
  {
    this.categoryId = categoryId;
  }


  protected int isProvidersAllowed(Long providerId)
  {
    if (this.providerId == null)
      return UNKNOWN;
    if (providerId.longValue() == -1)
      return UNKNOWN;
    if (this.providerId == providerId)
      return ALLOWED;


    return NOT_ALLOWED;
  }

  protected int isCategoriesAllowed(Long categoryId)
  {
    if (this.categoryId == null)
      return UNKNOWN;
    if (categoryId.longValue() == -1)
      return UNKNOWN;
    if (this.categoryId == categoryId)
      return ALLOWED;

    return NOT_ALLOWED;
  }

  public boolean isItemAllowed(DataItem item)
  {
    if (isEmpty())
      return true;
    StatRouteDataItem statitem = (StatRouteDataItem) item;
    Provider provider = (Provider) statitem.getValue("provider");
    Category category = (Category) statitem.getValue("category");
    long providerId = provider!=null?provider.getId():-1;
    long categoryId = category!=null?category.getId():-1;
    long filterProviderId = this.providerId != null ? this.providerId.longValue():-1;
    long filterCategoryId = this.categoryId != null ? this.categoryId.longValue():-1;
    boolean result = (filterProviderId == -1 || providerId == filterProviderId) && (filterCategoryId == -1 || categoryId == filterCategoryId);
/*    System.out.println("statfilter: fpi="+filterProviderId+
        " pi="+providerId+
        " fci="+filterCategoryId+
        " ci="+categoryId+
        " res = "+result
    );*/
    return result;

  }


}
