package ru.novosoft.smsc.admin.dl.exceptions;

import ru.novosoft.smsc.admin.AdminException;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 14.10.2003
 * Time: 15:27:22
 */
public class OwnerNotExistsException extends AdminException
{
  private final String dlname;
  private final String owner;

  public OwnerNotExistsException(String dlname, String owner)
  {
    super("Owner \"" + owner + "\" for list \"" + dlname + "\" not found");
    this.dlname = dlname;
    this.owner = owner;
  }

  public String getDlname()
  {
    return dlname;
  }

  public String getOwner()
  {
    return owner;
  }
}
