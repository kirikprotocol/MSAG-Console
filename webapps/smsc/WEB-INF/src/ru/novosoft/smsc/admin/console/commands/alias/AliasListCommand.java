/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 19, 2002
 * Time: 2:35:30 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.alias;

import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasFilter;

import java.util.Iterator;
import java.util.List;

public class AliasListCommand extends CommandClass {

  private String alias = null;
  private String address = null;
  private Integer hide = null;

  public void process(CommandContext ctx) {

    try {
      final AliasFilter filter = new AliasFilter();

      if (address != null)
        filter.setAddresses(new String[]{address});
      if (alias != null)
        filter.setAliases(new String[] {alias});
      if (hide != null) {
        if (hide.intValue() != 1 && hide.intValue() != 0)
          throw new AdminException("Wrong hide value. Must be 1 or 0");
        filter.setHide((byte)hide.intValue());
      }

      final List aliases = ctx.getSmsc().getAliases().findAliases(filter, 100);

      if (aliases.isEmpty()) {
        ctx.setMessage("No aliases found");
        ctx.setStatus(CommandContext.CMD_OK);
      } else {
        ctx.setMessage("Alias list");
        for (Iterator iter = aliases.iterator(); iter.hasNext() ;) {
          System.out.println("add");
          ctx.addResult(((Alias)iter.next()).getAlias().getMask());
        }
        ctx.setStatus(CommandContext.CMD_LIST);
      }


    } catch (AdminException e) {
      ctx.setMessage("Couldn't list aliases. Cause: "+e.getMessage());
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);

    }


//        try
//        {
//            Iterator i = ctx.getSmsc().getAliases().iterator();
//            if (!i.hasNext()) {
//                ctx.setMessage("No aliases defined");
//                ctx.setStatus(CommandContext.CMD_OK);
//            }
//            else {
//                while (i.hasNext()) {
//                    Alias alias = (Alias)i.next();
//                    if (alias != null) {
//                        ctx.addResult(alias.getAlias().getMask());
//                    }
//                }
//                ctx.setMessage("Alias list");
//                ctx.setStatus(CommandContext.CMD_LIST);
//            }
//        }
//        catch (Exception e) {
//            ctx.setMessage("Couldn't list aliases. Cause: "+e.getMessage());
//            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
//            return;
//        }
  }

  public String getId() {
    return "ALIAS_LIST";
  }

  public void setAlias(String alias) {
    this.alias = alias;
  }

  public void setAddress(String address) {
    this.address = address;
  }

  public void setHide(int hide) {
    this.hide = new Integer(hide);
  }
}
