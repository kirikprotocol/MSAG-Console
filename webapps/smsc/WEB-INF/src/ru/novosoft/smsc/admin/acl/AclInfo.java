package ru.novosoft.smsc.admin.acl;

import org.apache.log4j.Category;
import ru.novosoft.smsc.util.Functions;

import java.util.List;


/**
 * Created by igork
 * Date: 15.06.2004
 * Time: 19:23:09
 */
public class AclInfo
{
  public static final char ACT_UNDEFINED = '0';
  public static final char ACT_DBSDIRECT = '1';
  public static final char ACT_FULLCACHE = '2';

  private Category logger = Category.getInstance(this.getClass());

  protected long id;
  protected String name;
  protected String description;
  protected char cache_mode;


  public AclInfo(final long id, final String name, final String description, final char cache_mode)
  {
    this.cache_mode = cache_mode;
    if (name == null) throw new NullPointerException("name is null");
    if (description == null) throw new NullPointerException("description is null");

    this.id = id;
    this.name = name;
    this.description = description;
  }

  public AclInfo(List properties)
  {
    final String idStr = (String) properties.get(0);
    final String name = (String) properties.get(1);
    final String description = (String) properties.get(2);
    final String cache_mode_str = (String) properties.get(3);

    logger.debug("create AclInfo[" + idStr + ", \"" + name + ", \"" + description + "\"]");

    this.id = Long.parseLong(idStr);
    this.name = name;
    this.description = description;
    this.cache_mode = cache_mode_str != null && cache_mode_str.length() > 0 ? cache_mode_str.charAt(0) : ACT_UNDEFINED;
  }

  public AclInfo(final long id, final String name)
  {
    if (name == null) throw new NullPointerException("name is null");

    this.id = id;
    this.name = name;
    this.description = null;
    this.cache_mode = 0;
  }


  public int hashCode()
  {
    return (int) (id ^ (id >>> 32));
  }

  public boolean equals(Object obj)
  {
    if (obj instanceof Acl) {
      Acl acl = (Acl) obj;
      return acl.id == this.id
             && Functions.compareStrs(acl.name, this.name)
             && Functions.compareStrs(acl.description, this.description);
    } else
      return false;
  }


  public long getId()
  {
    return id;
  }

  public void setId(long id)
  {
    this.id = id;
  }

  public String getName()
  {
    return name;
  }

  public void setName(String name)
  {
    this.name = name;
  }

  public String getDescription()
  {
    return description;
  }

  public void setDescription(String description)
  {
    this.description = description;
  }

  public char getCache_mode()
  {
    return cache_mode;
  }

  public void setCache_mode(char cache_mode)
  {
    this.cache_mode = cache_mode;
  }
}
