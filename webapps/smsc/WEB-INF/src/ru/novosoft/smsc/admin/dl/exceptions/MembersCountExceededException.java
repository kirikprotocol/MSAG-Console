package ru.novosoft.smsc.admin.dl.exceptions;

import ru.novosoft.smsc.admin.AdminException;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 14.10.2003
 * Time: 15:03:48
 */
public class MembersCountExceededException extends AdminException
{
  private final String dlname;
  private final int actualMembersCount;
  private final int dlMaxMembers;

  public MembersCountExceededException(String dlname, int actualMembersCount, int dlMaxMembers)
  {
    super("List \"" + dlname + "\" have too many members (" + actualMembersCount + " of " + dlMaxMembers + ")");
    this.dlname = dlname;
    this.actualMembersCount = actualMembersCount;
    this.dlMaxMembers = dlMaxMembers;
  }

  public String getDlname()
  {
    return dlname;
  }

  public int getActualMembersCount()
  {
    return actualMembersCount;
  }

  public int getDlMaxMembers()
  {
    return dlMaxMembers;
  }
}
