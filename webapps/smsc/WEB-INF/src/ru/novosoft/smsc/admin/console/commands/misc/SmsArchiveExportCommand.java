package ru.novosoft.smsc.admin.console.commands.misc;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.smsexport.SmsExport;
import ru.novosoft.smsc.admin.smsstat.ExportSettings;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.lang.reflect.Method;

public class SmsArchiveExportCommand extends CommandClass {
    private final static String DATE_FORMAT = "dd.MM.yyyy";

    private ExportSettings export = new ExportSettings();
    private String dateStr = "";

    public void process(CommandContext ctx) {
        SmsExport smsExport = null;
        try {
            Date date = null;
            try {
                SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
                date = formatter.parse(dateStr);
            } catch (Exception e) {
                ctx.setMessage("Invalid date format '" + dateStr + "', expected format: " + DATE_FORMAT);
                ctx.setStatus(CommandContext.CMD_PARSE_ERROR);
                return;
            }

            Class archiveExportClass = Class.forName("ru.novosoft.smsc.admin.smsexport.SmsArchiveExport");
            Method getArchiveInstance = archiveExportClass.getMethod("getInstance", new Class[] {Config.class, SMSCAppContext.class});
            smsExport = (SmsExport) getArchiveInstance.invoke(null, new Object[] {ctx.getWebappConfig(), ctx.getOwner().getContext()});
            //smsExport = SmsArchiveExport.getInstance(ctx.getWebappConfig(), ctx.getOwner().getContext());
            Method setDate = archiveExportClass.getMethod("setDate", new Class[] {Date.class});
            setDate.invoke(smsExport, new Object[] {date});//((SmsArchiveExport) operative).setDate(date);
            //((SmsArchiveExport) smsExport).setDate(date);
            if (export == null || export.isEmpty())
                smsExport.export();
            else
                smsExport.export(export);
            ctx.setMessage("Exported");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (ClassNotFoundException e) {
            ctx.setMessage("Couldn't export messages. Cause: Feature is not installed");
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        } catch (Exception e) {
            ctx.setMessage("Couldn't export messages. Cause: Failed to get export instance. Details: " + e.getMessage());
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

    public void setDate(String dateStr) {
        this.dateStr = dateStr;
    }

    public String getId() {
        return "EXPORT_SMS";
    }
}

