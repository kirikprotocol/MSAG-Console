/*
 * Created by igork
 * Date: 05.11.2002
 * Time: 23:38:40
 */
package ru.novosoft.smsc.jsp.smsc.routes;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.jsp.util.tables.impl.route.RouteFilter;
import ru.novosoft.smsc.util.Functions;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

public class RoutesFilter extends SmscBean
{
  protected RouteFilter filter = null;

  protected String[] srcChks = null;
  protected String[] srcMasks = null;
  protected String[] dstChks = null;
  protected String[] dstMasks = null;
  protected String[] smeChks = null;
  private String[] names = null;

  protected boolean strict = false;
  protected int strict1 = 2;
  protected boolean showSrc = false;
  protected boolean showDst = false;

  protected String filteredName = null;
  protected String queryProvider = null;
  protected String queryCategory = null;

  protected Set srcChksSet = null;
  protected Set dstChksSet = null;
  protected Set smeChksSet = null;

  protected String mbApply = null;
  protected String mbClear = null;
  protected String mbCancel = null;

  protected boolean initialized = false;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    filter = preferences.getRoutesFilter();

    if (srcMasks == null || dstMasks == null) {
      srcChks = filter.getSourceSubjectNames();
      srcMasks = filter.getSourceMaskStrings();
      dstChks = filter.getDestinationSubjectNames();
      dstMasks = filter.getDestinationMaskStrings();
      smeChks = filter.getSmeIds();
      names=filter.getNames();
      strict = filter.isIntersection();
      strict1 = filter.getIntersection();
      showSrc = preferences.isRouteShowSrc();
      showDst = preferences.isRouteShowDst();
    }

    if (srcChks == null) srcChks = new String[0];
    if (dstChks == null) dstChks = new String[0];
    if (smeChks == null) smeChks = new String[0];

    srcChks = Functions.trimStrings(srcChks);
     if (!initialized) {
    try {
          filteredName=filter.getNames()[0];
        } catch (Exception e) {
          filteredName="";
        }
     try {
          queryProvider=filter.getProviders()[0];
        } catch (Exception e) {
          queryProvider="";
        }
        try {
          queryCategory=filter.getCategories()[0];
        } catch (Exception e) {
          queryCategory="";
        }
     }
    try {
      srcMasks = MaskList.normalizeMaskList(Functions.trimStrings(srcMasks));
    } catch (AdminException e) {
      return error(SMSCErrors.error.routes.invalidSourceMask, e);
    }
    dstChks = Functions.trimStrings(dstChks);
    try {
      dstMasks = MaskList.normalizeMaskList(Functions.trimStrings(dstMasks));
    } catch (AdminException e) {
      return error(SMSCErrors.error.routes.invalidDestinationMask, e);
    }
    smeChks = Functions.trimStrings(smeChks);

    srcChksSet = new HashSet(Arrays.asList(srcChks));
    dstChksSet = new HashSet(Arrays.asList(dstChks));
    smeChksSet = new HashSet(Arrays.asList(smeChks));

    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    if (mbCancel != null)
      return RESULT_DONE;

    int result = super.process(request);

    if (mbClear != null) {
       int dresult = clearFilter();
      return (dresult != RESULT_OK) ? dresult : RESULT_DONE;
    }


  /*  if (mbClear != null) {
      filter = preferences.getRoutesFilter();

      srcChks = srcMasks = dstChks = dstMasks = smeChks = new String[0];
      strict = showSrc = showDst = false;
      strict1=2;filteredName=""; queryProvider=""; queryCategory="";
      srcChksSet = dstChksSet = smeChksSet = new HashSet();
      mbApply = mbClear = mbCancel = null;
      filter.setNames(new String[0]);
      filter.setProviders(new String[0]);
      filter.setCategories(new String[0]);
      this.errors.clear();

      return RESULT_OK;
    } */
    else
    if (result == RESULT_OK && mbApply != null) {
     initialized = true;

       if (filteredName != null)
            filter.setNames(new String[]{filteredName.toLowerCase()});
        else
            filter.setNames(new String[0]);
      if (queryProvider != null)
           filter.setProviders(new String[]{queryProvider.toLowerCase()});
        else
          filter.setProviders(new String[0]);
        if (queryCategory != null)
           filter.setCategories(new String[]{queryCategory.toLowerCase()});
        else
          filter.setCategories(new String[0]);
      try {
        filter.setSourceMaskStrings(srcMasks);
      } catch (AdminException e) {
        return error(SMSCErrors.error.routes.invalidSourceMask, e);
      }
      try {
        filter.setDestinationMaskStrings(dstMasks);
      } catch (AdminException e) {
        return error(SMSCErrors.error.routes.invalidDestinationMask, e);
      }
      filter.setSourceSubjectNames(srcChks);
      filter.setDestinationSubjectNames(dstChks);
      filter.setSmeIds(smeChks);

     /*  int strict1;
      if (strict) strict1=0;
       else strict1=2;
       */

      filter.setIntersection(strict1);
      preferences.setRouteShowSrc(showSrc);
      preferences.setRouteShowDst(showDst);
      return RESULT_DONE;
    }

    return result;
  }
     private int clearFilter() {
       try {
    /*         srcChks = srcMasks = dstChks = dstMasks = smeChks = new String[0];
      strict = showSrc = showDst = false;
      strict1=2;filteredName=""; queryProvider=""; queryCategory="";
      srcChksSet = dstChksSet = smeChksSet = new HashSet();
      mbApply = mbClear = mbCancel = null;
      */
            filter = preferences.getRoutesFilter();
        // filter.setIntersection(false);
             filter.setNames(new String[0]);
             filter.setProviders(new String[0]);
             filter.setCategories(new String[0]);
             filter.setSourceSubjectNames(new String[0]);
             filter.setDestinationSubjectNames(new String[0]);
             filter.setSourceMaskStrings(new String[0]);
             filter.setDestinationMaskStrings(new String[0]);
             filter.setSmeIds(new String[0]);
         //this.errors.clear();
       } catch (AdminException e) {
         return error(SMSCErrors.error.routes.CantUpdateFilter, e);
       }
         return RESULT_OK;
     }

  public boolean isSrcChecked(String subj)
  {
    return srcChksSet.contains(subj);
  }

  public boolean isDstChecked(String subj)
  {
    return dstChksSet.contains(subj);
  }

  public boolean isSmeChecked(String sme)
  {
    return smeChksSet.contains(sme);
  }

  public Collection getAllSubjects()
  {
    return routeSubjectManager.getSubjects().getNames();
  }

  public Collection getAllSmes()
  {
    return smeManager.getSmeNames();
  }


  /***************************** properties **********************************/
  public String[] getSrcChks()
  {
    return srcChks;
  }

  public void setSrcChks(String[] srcChks)
  {
    this.srcChks = srcChks;
  }

  public String[] getSrcMasks()
  {
    return srcMasks;
  }

  public void setSrcMasks(String[] srcMasks)
  {
    this.srcMasks = srcMasks;
  }

  public String[] getDstChks()
  {
    return dstChks;
  }

  public void setDstChks(String[] dstChks)
  {
    this.dstChks = dstChks;
  }

  public String[] getDstMasks()
  {
    return dstMasks;
  }

  public void setDstMasks(String[] dstMasks)
  {
    this.dstMasks = dstMasks;
  }

  public String[] getSmeChks()
  {
    return smeChks;
  }

  public void setSmeChks(String[] smeChks)
  {
    this.smeChks = smeChks;
  }

  public boolean isStrict()
  {
    return strict;
  }

  public int getStrict1()
  {
    return strict1;
  }

  public void setStrict1(int strict1)
  {
    this.strict1 = strict1;
  }

  public void setStrict(boolean strict)
  {
    this.strict = strict;
  }

  public String getQueryCategory()
  {
    return queryCategory;
  }

  public void setQueryCategory(String queryCategory)
  {
    this.queryCategory = queryCategory;
  }

  public String getFilteredName()
  {
    return filteredName;
  }

  public void setFilteredName(String filteredName)
  {
    this.filteredName = filteredName;
  }

  public String getQueryProvider()
  {
    return queryProvider;
  }

  public void setQueryProvider(String queryProvider)
  {
    this.queryProvider = queryProvider;
  }

  public boolean isInitialized()
  {
    return initialized;
  }

  public void setInitialized(boolean initialized)
  {
    this.initialized = initialized;
  }

  public boolean isShowSrc()
  {
    return showSrc;
  }

  public void setShowSrc(boolean showSrc)
  {
    this.showSrc = showSrc;
  }

  public boolean isShowDst()
  {
    return showDst;
  }

  public void setShowDst(boolean showDst)
  {
    this.showDst = showDst;
  }

  public Set getSmeChksSet()
  {
    return smeChksSet;
  }

  public void setSmeChksSet(Set smeChksSet)
  {
    this.smeChksSet = smeChksSet;
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

  public String getMbClear()
  {
    return mbClear;
  }

  public void setMbClear(String mbClear)
  {
    this.mbClear = mbClear;
  }
}
