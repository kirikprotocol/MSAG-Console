package ru.novosoft.smsc.admin.dl.exceptions;

import ru.novosoft.smsc.admin.AdminException;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 14.10.2003
 * Time: 17:06:52
 */
public class ListsCountExceededException extends AdminException
{
  private final String owner;

  public ListsCountExceededException(String owner)
  {
    super("Lists count exceeded for owner \"" + owner + '"');
    this.owner = owner;
  }

  public String getOwner()
  {
    return owner;
  }
}
