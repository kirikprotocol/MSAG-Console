/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 20, 2003
 * Time: 3:06:00 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.jsp.dl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.Service;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.dl.DistributionList;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.DistributionListManager;
import ru.novosoft.smsc.admin.dl.exceptions.ListNotExistsException;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

public class DistributionListAdminFormBean extends IndexBean
{
  public static final int RESULT_EDIT = IndexBean.PRIVATE_RESULT;
  public static final int RESULT_ADD = IndexBean.PRIVATE_RESULT + 1;
  protected static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 2;

  private DistributionListAdmin admin = null;
  private List dls = new ArrayList();
  private String[] checkedDls = new String[0];
  private Set checkedDlsSet = null;
  private String editDl = null;

  private String mbEdit = null;
  private String mbDelete = null;
  private String mbAdd = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (this.admin == null)
      admin = appContext.getSmsc().getDistributionListAdmin();

    if (pageSize == 0)
      pageSize = 20;
    if (sort == null || sort.length() == 0)
      sort = "name";

    if (checkedDls == null)
      checkedDls = new String[0];
    checkedDlsSet = new HashSet(Arrays.asList(checkedDls));
    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;
    //query names
    try {

      dls = admin.list(preferences.getDlFilter());
    } catch (AdminException e) {
      return error(SMSCErrors.error.dl.CouldntListDistributionLists);
    }

    totalSize = dls.size();
    sortLists(dls);
    dls = dls.subList(startPosition, Math.min(startPosition + pageSize, dls.size()));
    if (sort.startsWith("-"))
      Collections.reverse(dls);

    if (mbEdit != null)
      return RESULT_EDIT;
    else if (mbDelete != null)
      return delete();
    else if (mbAdd != null)
      return RESULT_ADD;

    return result;
  }

  private void sortLists(List dls)
  {
    final boolean sortByName = "name".equals(sort);

    Collections.sort(dls, new Comparator()
    {
      public int compare(Object o1, Object o2)
      {
        if (o1 instanceof DistributionList && o2 instanceof DistributionList) {
          DistributionList d1 = (DistributionList) o1;
          DistributionList d2 = (DistributionList) o2;
          return sortByName
                  ? d1.getName().compareToIgnoreCase(d2.getName())
                  : (d1.getOwner() != null
                  ? (d2.getOwner() != null ? d1.getOwner().compareToIgnoreCase(d2.getOwner()) : 1)
                  : (d2.getOwner() == null ? 0 : -1)
                  );
        } else
          return 0;
      }
    });
  }

  private int delete()
  {
    int result = RESULT_DONE;
    for (int i = 0; i < checkedDls.length; i++) {
      String checkedDl = checkedDls[i];
      try {
        admin.deleteDistributionList(checkedDl);
        checkedDlsSet.remove(checkedDl);
        //dls.remove(checkedDl);
        journalAppend(SubjectTypes.TYPE_dl, checkedDl, Actions.ACTION_DEL);
        logger.debug("Distribution list \"" + checkedDl + "\" deleted");
      } catch (ListNotExistsException e) {
        logger.error("Couldn't delete distribution list \"" + checkedDl + "\": unknown DL");
        result = error(SMSCErrors.error.dl.unknownDL, checkedDl);
      } catch (AdminException e) {
        logger.error("Couldn't delete distribution list \"" + checkedDl + '"');
        result = error(SMSCErrors.error.dl.couldntdelete, checkedDl);
      }
    }
    return result;
  }

  public boolean isDlChecked(String name)
  {
    return checkedDlsSet.contains(name);
  }

  public List getDls()
  {
    return dls;
  }

  public String[] getCheckedDls()
  {
    return checkedDls;
  }

  public void setCheckedDls(String[] checkedDls)
  {
    this.checkedDls = checkedDls;
  }

  public String getEditDl()
  {
    return editDl;
  }

  public void setEditDl(String editDl)
  {
    this.editDl = editDl;
  }

  public String getMbEdit()
  {
    return mbEdit;
  }

  public void setMbEdit(String mbEdit)
  {
    this.mbEdit = mbEdit;
  }

  public String getMbDelete()
  {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete)
  {
    this.mbDelete = mbDelete;
  }

  public String getMbAdd()
  {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd)
  {
    this.mbAdd = mbAdd;
  }
}
