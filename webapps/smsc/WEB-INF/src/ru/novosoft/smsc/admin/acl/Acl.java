package ru.novosoft.smsc.admin.acl;

import java.util.*;


/**
 * Created by igork
 * Date: 10.06.2004
 * Time: 15:58:27
 */
public class Acl extends AclInfo
{
  private final Set addresses;

  public Acl(final long id, final String name, final String description, final Set addresses)
      throws NullPointerException
  {
    super(id, name, description);

    if (addresses == null) throw new NullPointerException("addresses is null");

    this.addresses = addresses;
  }

  public Acl(List properties)
      throws NumberFormatException
  {
    super(Long.parseLong((String) properties.get(0)), (String) properties.get(1), (String) properties.get(2));
    this.addresses = new HashSet();
  }


  public boolean equals(Object obj)
  {
    if (obj instanceof Acl) {
      Acl acl = (Acl) obj;
      return super.equals(obj)
             && acl.addresses.equals(this.addresses);
    } else
      return false;
  }


  public Set getAddresses()
  {
    return addresses;
  }
}
