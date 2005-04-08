package ru.novosoft.smsc.jsp.util.tables.impl.smcstat;

/**
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;

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
    Long providerId = (Long) statitem.getValue("providerId");
    Long categoryId = (Long) statitem.getValue("categoryId");

    boolean result = ((this.providerId == null || providerId == null || this.providerId.longValue() == providerId.longValue())
            && (this.categoryId == null || categoryId == null || this.categoryId.longValue() == categoryId.longValue()));
    System.out.println("statfilter: t.pi="+(this.providerId==null?"null":this.providerId.toString())+
        " pi="+(providerId==null?"null":providerId.toString())+
        " t.ci="+(this.categoryId==null?"null":this.categoryId.toString())+
        " ci="+(categoryId==null?"null":categoryId.toString())
    );
    return result;

  }


}
