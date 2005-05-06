package ru.novosoft.smsc.admin.console.commands.misc;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 05.05.2005
 * Time: 19:08:47
 * To change this template use File | Settings | File Templates.
 */
public class SmsExportCommand implements Command
{
    // export messages [to driver source user pass table_name]

    public void process(CommandContext ctx)
    {
        try {
            throw new Exception("Not implemented yet");
            //ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage("Couldn't export messages. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public void setSource(String source) {

    }
    public void setDriver(String driver) {

    }
    public void setUser(String user) {

    }
    public void setPassword(String password) {

    }
    public void setTableName(String tableName) {

    }

    public String getId() {
        return "EXPORT_SMS";
    }

}

