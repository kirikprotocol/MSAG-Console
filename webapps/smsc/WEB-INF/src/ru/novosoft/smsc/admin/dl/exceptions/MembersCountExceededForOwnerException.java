package ru.novosoft.smsc.admin.dl.exceptions;

import ru.novosoft.smsc.admin.AdminException;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 15.10.2003
 * Time: 18:31:55
 */
public class MembersCountExceededForOwnerException extends AdminException
{
  private final String principal;
  private final int actualMaxMembers;
  private final int maxMembers;

  public MembersCountExceededForOwnerException(String principal, int actualMaxMembers, int maxMembers)
  {
    super("Principal \"" + principal + "\" have to many members in one of them lists (" + actualMaxMembers + " of " + maxMembers +')');
    this.principal = principal;
    this.actualMaxMembers = actualMaxMembers;
    this.maxMembers = maxMembers;
  }

  public String getPrincipal()
  {
    return principal;
  }

  public int getActualMaxMembers()
  {
    return actualMaxMembers;
  }

  public int getMaxMembers()
  {
    return maxMembers;
  }
}
