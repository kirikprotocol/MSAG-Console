package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.acl.AclInfo;

import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 06.07.2004
 * Time: 14:39:38
 * To change this template use File | Settings | File Templates.
 */
public class AclListCommand implements Command
{
  public void process(CommandContext ctx)
  {
    try
    {
      List list = ctx.getSmsc().aclListNames();
      if (list == null || list.size() <= 0) {
        ctx.setMessage("No ACLs defined");
        ctx.setStatus(CommandContext.CMD_OK);
      } else {
        ctx.setStatus(CommandContext.CMD_LIST);
        ctx.setMessage("ACLs list");
        for (int i=0; i<list.size(); i++) {
          AclInfo info = (AclInfo)list.get(i);
          ctx.addResult(""+info.getId()+" '"+info.getName()+"'");
        }
      }
    } catch (Exception e) {
      ctx.setMessage("Couldn't list ACLs. Cause: "+e.getMessage());
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public String getId() {
    return "ACL_LIST";
  }
}
