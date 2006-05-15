package ru.novosoft.smsc.admin.console.commands.misc;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.smsexport.SmsExport;
import ru.novosoft.smsc.admin.smsexport.SmsOperativeExport;
import ru.novosoft.smsc.admin.smsstat.ExportSettings;

public class SmsExportCommand extends CommandClass {
    private ExportSettings export = new ExportSettings();

    public void process(CommandContext ctx) {
        SmsExport smsExport = null;
        try {
            smsExport = SmsOperativeExport.getInstance(ctx.getWebappConfig(), ctx.getSmscConfig());

            if (export == null || export.isEmpty())
                smsExport.export();
            else
                smsExport.export(export);
            ctx.setMessage("Exported");
            ctx.setStatus(CommandContext.CMD_OK);
        }
        catch (Exception e) {
            ctx.setMessage("Couldn't export messages. Cause: " + e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public void setSource(String source) {
        export.setSource(source);
    }

    public void setDriver(String driver) {
        export.setDriver(driver);
    }

    public void setUser(String user) {
        export.setUser(user);
    }

    public void setPassword(String password) {
        export.setPassword(password);
    }

    public void setTableName(String tableName) {
        export.setTablesPrefix(tableName);
    }

    public String getId() {
        return "EXPORT_SMS";
    }
}

