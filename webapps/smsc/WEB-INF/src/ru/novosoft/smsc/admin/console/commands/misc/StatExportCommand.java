package ru.novosoft.smsc.admin.console.commands.misc;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.smsstat.ExportSettings;
import ru.novosoft.smsc.admin.smsstat.SmsStat;
import ru.novosoft.smsc.admin.smsstat.StatQuery;
import ru.novosoft.smsc.admin.smsstat.ExportResults;

import java.util.Date;
import java.util.Calendar;
import java.util.TimeZone;
import java.text.SimpleDateFormat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 05.05.2005
 * Time: 19:03:29
 * To change this template use File | Settings | File Templates.
 */
public class StatExportCommand implements Command
{
    // export stats for 25.04.2005 [to driver source user pass table_prefix]
    private final static String DATE_FORMAT = "dd.MM.yyyy";
    private Calendar localCaledar = Calendar.getInstance(TimeZone.getDefault());

    private String dateStr = null;
    private ExportSettings export = new ExportSettings();

    public void process(CommandContext ctx)
    {
        Date date = null;
        try {
            SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
            date = formatter.parse(dateStr);
        } catch (Exception e) {
            ctx.setMessage("Invalid date format '"+dateStr+"', expected format: "+DATE_FORMAT);
            ctx.setStatus(CommandContext.CMD_PARSE_ERROR);
            return;
        }
        SmsStat stat = null;
        try {
            stat = SmsStat.getInstance(ctx.getSmscConfig(), ctx.getWebappConfig());
        } catch(Exception e) {
            ctx.setMessage("Failed to get stat instance. Details: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }

        try
        {
            ExportResults results = new ExportResults();
            StatQuery query = new StatQuery();
            localCaledar.setTime(date);
            localCaledar.set(Calendar.HOUR_OF_DAY, 0); localCaledar.set(Calendar.MINUTE, 0);
            localCaledar.set(Calendar.SECOND, 0); localCaledar.set(Calendar.MILLISECOND, 0);
            Date fromDate = localCaledar.getTime();
            query.setFromDate(fromDate); query.setFromDateEnabled(true);
            localCaledar.add(Calendar.DAY_OF_MONTH, 1);
            Date tillDate = localCaledar.getTime();
            query.setTillDate(tillDate); query.setTillDateEnabled(true);

            if (export == null || export.isEmpty())
                 stat.exportStatistics(query, results);
            else stat.exportStatistics(query, results, export);
            ctx.setMessage("Exported: total "+results.total.records+"/"+results.total.errors+
                           ", smes "+results.smes.records+"/"+results.smes.errors+
                           ", routes "+results.routes.records+"/"+results.routes.errors);
            ctx.setStatus(CommandContext.CMD_OK);
        }
        catch (Exception e) {
            ctx.setMessage("Couldn't export statistics. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public void setDate(String dateStr) {
        this.dateStr = dateStr;
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
    public void setTablesPrefix(String tablesPrefix) {
        export.setTablesPrefix(tablesPrefix);
    }

    public String getId() {
        return "EXPORT_STATS";
    }

}
