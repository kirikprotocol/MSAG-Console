/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Jan 17, 2003
 * Time: 7:37:52 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.alias;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasDataSource;

import java.util.List;

public class AliasShowCommand extends CommandClass
{
    private String address = null;

    public void setAddress(String address) {
        this.address = address;
    }

    public void process(CommandContext ctx)
    {
        String out = "for address '"+address+"'";
        try
        {
            final Mask addrMask = new Mask(address);
//            Iterator i = ctx.getSmsc().getAliases().iterator();
//            if (!i.hasNext()) {
//                ctx.setMessage("No aliases defined at all");
//                ctx.setStatus(CommandContext.CMD_OK);
//                return;
//            }
//            while (i.hasNext()) {
//                Alias alias = (Alias)i.next();
//                String mask = alias.getAddress().getMask();
//                if (mask.equals(addrMask.getMask())) {
//                    ctx.addResult(alias.getAlias().getMask());
//                }
//            }
            final Filter filter = new Filter() {

              public boolean isEmpty() { return false; }

              public boolean isItemAllowed(DataItem item) {
                try {
                  return new Mask((String)item.getValue(AliasDataSource.ADDRESS_FIELD)).getMask().equals(addrMask.getMask());
                } catch (AdminException e) {
                  e.printStackTrace();
                  return false;
                }
              }
            };

            final List aliases = ctx.getSmsc().getAliases().findAliases(filter, 100);
            for (int i = 0; i < aliases.size(); i++) {
              Alias alias = (Alias) aliases.get(i);
              ctx.addResult(alias.getAlias().getMask());
            }
            if (ctx.getResults().size() > 0) {
                ctx.setStatus(CommandContext.CMD_LIST);
                ctx.setMessage("Alias list "+out);
            } else {
                ctx.setStatus(CommandContext.CMD_OK);
                ctx.setMessage("No aliases defined "+out);
            }
        }
        catch (Exception e) {
            ctx.setMessage("Couldn't show aliases "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            return;
        }
    }

    public String getId() {
        return "ALIAS_SHOW";
    }
}
