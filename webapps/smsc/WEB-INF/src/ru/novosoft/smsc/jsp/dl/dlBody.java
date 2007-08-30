package ru.novosoft.smsc.jsp.dl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.SortedList;

import javax.servlet.http.HttpServletRequest;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 * Created by igork
 * Date: 26.02.2003
 * Time: 18:36:25
 */
public abstract class dlBody extends IndexBean {
  protected String name = null;
  protected String owner = null;
  protected boolean system = true;
  protected int maxElements = 0;
  protected String[] submitters = null;
  protected String[] members = null; //receivers
  protected List fullMembersList = null;
  protected List fullSubmittersList = null;
  protected boolean initialized = false;
  protected boolean isChanged = false;

  private void processAddresses(List fullList, String[] newList) throws AdminException {
    newList = Functions.trimStrings(newList);
    fullList.clear();
    for (int i = 0; i < newList.length; i++) {
      fullList.add(newList[i]);
    }
  }

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (name == null) {
      name = "";
      owner = "";
      maxElements = 0;
    }

    return RESULT_OK;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    try {
      if ((result = processSubmitters()) != RESULT_OK)
        return result;
      if ((result = processMembers()) != RESULT_OK)
        return result;
    } catch (AdminException e) {
      logger.error("Errors in entered data", e);
      return error(SMSCErrors.error.dl.invalidAddress, e);
    }

/*    if (initialized && fullSubmittersList.size() == 0)
      return error(SMSCErrors.error.dl.noSubmittersEntered);
    if (initialized && fullMembersList.size() == 0)
      return error(SMSCErrors.error.dl.noMembersEntered);*/

    return RESULT_OK;
  }

  private int processSubmitters() throws AdminException {
    if (isChanged) processAddresses(fullSubmittersList, submitters);

    submitters = (String[]) new SortedList(fullSubmittersList).toArray(new String[0]);

    return RESULT_OK;
  }

  private int processMembers() throws AdminException {
    if (pageSize == 0)
      pageSize = preferences.getDlPageSize();
    else
      preferences.setDlPageSize(pageSize);

    if (sort == null || sort.length() == 0)
      sort = preferences.getDlSortOrder();
    else
      preferences.setDlSortOrder(sort);

    sort(fullMembersList, sort);

//    if (isChanged)
    if (members != null)
        processAddresses(fullMembersList, members);

    totalSize = fullMembersList.size();
    List newMembersList = fullMembersList;//.subList(Math.max(startPosition, 0), Math.min(startPosition + pageSize, fullMembersList.size()));
    members = (String[]) newMembersList.toArray(new String[0]);
    return RESULT_OK;
  }

  private static void sort(List listToSort, String sortOption) {
    //sort members
    final boolean isNegativeSort = sortOption.startsWith("-");
    Collections.sort(listToSort, new Comparator() {
      public int compare(Object o1, Object o2) {
        String s1 = (String) o1;
        String s2 = (String) o2;
        return isNegativeSort ? s2.compareTo(s1) : s1.compareTo(s2);
      }
    });
  }


  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name.toLowerCase();
  }

  public String getOwner() {
    return owner;
  }

  public void setOwner(String owner) {
    this.owner = owner;
  }

  public String getMaxElements() {
    return Integer.toString(maxElements);
  }

  public void setMaxElements(String maxElements) {
    try {
      this.maxElements = Integer.decode(maxElements).intValue();
    } catch (NumberFormatException e) {
      this.maxElements = 0;
    }
  }

  public String[] getSubmitters() {
    return submitters;
  }

  public void setSubmitters(String[] submitters) {
    this.submitters = submitters;
  }

  public String[] getMembers() {
    return members;
  }

  public void setMembers(String[] members) {
    this.members = members;
  }

  protected void clear() {
    super.clear();
    name = owner = null;
    system = true;
    submitters = members = null;
    fullMembersList = fullSubmittersList = null;
    maxElements = 0;
    initialized = false;
    isChanged = false;
  }

  public boolean isSystem() {
    return system;
  }

  public void setSystem(boolean system) {
    this.system = system;
  }

  public boolean getIsChanged() {
    return isChanged;
  }

  public void setIsChanged(boolean changed) {
    isChanged = changed;
  }

  public abstract boolean isCreate();
}
