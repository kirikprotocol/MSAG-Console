package ru.novosoft.smsc.jsp.smsstat;

import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.jsp.util.tables.impl.smcstat.StatRouteFilter;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

/**
 * Created by andrey Date: 07.02.2005 Time: 12:45:18
 */
public class StatRoutesFilter extends SmscBean
{
  private StatRouteFilter filter = null;

  private Long providerId = null;
  private Long categoryId = null;
  private String initialized = null;

  private String mbApply = null;
  private String mbClear = null;
  private String mbCancel = null;


  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    filter = preferences.getStatRouteFilter();

    if (initialized == null) {
      providerId = filter.getProviderId();
      categoryId = filter.getCategoryId();
    }
    //if (providerId == null)  providerId =new Long(-1);
    //if (categoryId == null)  categoryId =new Long(-1) ;

    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbApply != null) {
      filter.setProviderId(providerId);
      filter.setCategoryId(categoryId);
      return RESULT_DONE;
    }
    else if (mbClear != null) {
      providerId = null;
      categoryId = null;
      return RESULT_OK;
    }
    else if (mbCancel != null)
      return RESULT_DONE;

    return RESULT_OK;
  }

  public StatRouteFilter getFilter()
  {
    return filter;
  }

  public void setFilter(StatRouteFilter filter)
  {
    this.filter = filter;
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
