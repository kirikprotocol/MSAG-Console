package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.acl.AclInfo;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 06.07.2004
 * Time: 14:11:25
 * To change this template use File | Settings | File Templates.
 */
public abstract class AclGenCommand implements Command
{
  public final static char CACHE_UNDEFINED = AclInfo.ACT_UNDEFINED;
  public final static char CACHE_DBSDIRECT = AclInfo.ACT_DBSDIRECT;
  public static final char CACHE_FULLCACHE = AclInfo.ACT_FULLCACHE;

  protected long   aclId = 0;
  protected String name = null;
  protected String description = "";
  protected String address = null;
  protected char   cache = CACHE_UNDEFINED;

  protected boolean isDescription = false;
  protected boolean isAddress = false;
  protected boolean isCache = false;
  protected boolean isName = false;

  public void setAclId(long aclId) {
    this.aclId = aclId;
  }
  public void setName(String name) {
    this.name = name; isName = true;
  }
  public void setDescription(String description) {
    this.description = description; isDescription = true;
  }
  public void setAddress(String address) {
    this.address = address; isAddress = true;
  }
  public void setCache(char cache) {
    this.cache = cache; isCache = true;
  }
}
