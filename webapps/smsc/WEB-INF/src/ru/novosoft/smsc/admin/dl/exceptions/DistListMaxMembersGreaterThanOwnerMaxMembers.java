package ru.novosoft.smsc.admin.dl.exceptions;

import ru.novosoft.smsc.admin.AdminException;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 14.10.2003
 * Time: 15:28:57
 */
public class DistListMaxMembersGreaterThanOwnerMaxMembers extends AdminException
{
  private final String dlname;
  private final String owner;
  private final int dlMaxElements;
  private final int ownerMaxMembers;

  public DistListMaxMembersGreaterThanOwnerMaxMembers(String dlname, String owner, int dlMaxElements, int ownerMaxMembers)
  {
    super("Max members limit (" + dlMaxElements + ") in list \"" + dlname + "\" greater than max members limit (" + ownerMaxMembers + ") in list owner \"" + owner + "\"");
    this.dlname = dlname;
    this.owner = owner;
    this.dlMaxElements = dlMaxElements;
    this.ownerMaxMembers = ownerMaxMembers;
  }

  public String getDlname()
  {
    return dlname;
  }

  public String getOwner()
  {
    return owner;
  }

  public int getDlMaxElements()
  {
    return dlMaxElements;
  }

  public int getOwnerMaxMembers()
  {
    return ownerMaxMembers;
  }
}
