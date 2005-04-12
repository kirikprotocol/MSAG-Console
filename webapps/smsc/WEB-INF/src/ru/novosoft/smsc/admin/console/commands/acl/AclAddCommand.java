package ru.novosoft.smsc.admin.console.commands.acl;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.AdminException;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 06.07.2004
 * Time: 14:38:32
 * To change this template use File | Settings | File Templates.
 */
public class AclAddCommand extends AclGenCommand
{
  public void process(CommandContext ctx)
  {
    String out = "ACL '"+name+"'";
    try
    {
      List addresses = new ArrayList();
      aclId = ctx.getSmsc().aclCreate(name, description, addresses, cache);
      ctx.setMessage(out+" added. New ACL id="+aclId);
      ctx.setStatus(CommandContext.CMD_OK);
    } catch (AdminException e) {
      ctx.setMessage("Couldn't add "+out+". Cause: "+e.getMessage());
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public String getId() {
    return "ACL_ADD";
  }
}
