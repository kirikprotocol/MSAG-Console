package ru.sibinco.scag.beans;

import ru.sibinco.lib.backend.util.SortByPropertyComparator;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.bean.TabledBean;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.*;


/**
 * Created by igork
 * Date: 09.03.2004
 * Time: 14:29:00
 */
public abstract class TabledBeanImpl extends SCAGBean implements TabledBean
{
  protected List tabledItems = new ArrayList();
  protected String sort = null;
  protected int pageSize = 25;
  protected int totalSize = 0;
  protected int startPosition = 0;
  protected String[] checked = new String[0];
  protected Set checkedSet = new HashSet();

  protected String mbEdit = null;
  protected String mbAdd = null;
  protected String mbDelete = null;

  public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException
  {
    super.process(request, response);
    if (mbEdit != null)
      throw new EditException(mbEdit);
    if (mbAdd != null)
      throw new AddException();
    if (mbDelete != null)
      delete();

    if (sort == null)
      sort = getDefaultSort();
    final SortedList results = new SortedList(getDataSource(), new SortByPropertyComparator(sort));
    totalSize = results.size();
    if (totalSize > startPosition)
      tabledItems = results.subList(startPosition, Math.min(totalSize, startPosition + pageSize));
    else
      tabledItems = new LinkedList();

  }

  protected abstract Collection getDataSource();

  protected String getDefaultSort()
  {
    return "id";
  }

  protected abstract void delete() throws SCAGJspException;

  public final List getTabledItems()
  {
    return tabledItems;
  }

  public final String getSort()
  {
    return sort;
  }

  public final void setSort(String sort)
  {
    this.sort = sort;
  }

  public final int getPageSize()
  {
    return pageSize;
  }

  public final void setPageSize(int pageSize)
  {
    this.pageSize = pageSize;
  }

  public final int getStartPosition()
  {
    return startPosition;
  }

  public final void setStartPosition(int startPosition)
  {
    this.startPosition = startPosition;
  }

  public final int getTotalSize()
  {
    return totalSize;
  }

  public final String[] getChecked()
  {
    return checked;
  }

  public final void setChecked(String[] checked)
  {
    this.checked = checked;
    this.checkedSet.clear();
    this.checkedSet.addAll(Arrays.asList(checked));
  }

  public final boolean isChecked(String id)
  {
    return checkedSet.contains(id);
  }

  protected final void setTabledItems(List tabledItems)
  {
    this.tabledItems = tabledItems;
  }

  protected final void setTotalSize(int totalSize)
  {
    this.totalSize = totalSize;
  }

  public String getMbEdit()
  {
    return mbEdit;
  }

  public void setMbEdit(String mbEdit)
  {
    this.mbEdit = mbEdit;
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
}
