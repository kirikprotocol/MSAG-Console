package ru.novosoft.smsc.admin.smsexport;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsstat.ExportSettings;
import ru.novosoft.smsc.admin.smsview.SmsSource;
import ru.novosoft.smsc.util.config.Config;
import ru.sibinco.util.conpool.ConnectionPool;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Properties;
import java.util.Date;
import java.util.TimeZone;
import java.text.SimpleDateFormat;

public abstract class SmsExport extends SmsSource {
    protected final static String DATE_FORMAT = "dd.MM.yyyy";
    protected SimpleDateFormat dateFormat = new SimpleDateFormat(DATE_FORMAT);

    protected final static String INSERT_OP_SQL = "INSERT INTO ";
    protected final static String UPDATE_OP_SQL = "UPDATE ";
    protected final static String DELETE_OP_SQL = "TRUNCATE TABLE ";
    protected final static String WHERE_SMS_ID_SQL = " WHERE ID=?";
    protected final static String CALL_multinsert_sms_SQL = "{ call multinsert_sms(?,?) }"; //1-integer , 2-msgs in arraylist
    protected final static String CREATE_PROC_INSERT_RECORD = " create or replace procedure insert_rec( msg in sms ) is begin ";

    protected final static String INSERT_FIELDS =
            "  (id,st,submit_time,valid_time,attempts,last_result, " +
                    "   last_try_time,next_try_time,oa,da,dda,mr,svc_type,dr,br,src_msc,     " +
                    "   src_imsi,src_sme_n,dst_msc,dst_imsi,dst_sme_n,route_id,svc_id,prty,  " +
                    "   src_sme_id,dst_sme_id,msg_ref,seq_num,arc,body_len) ";
    protected final static String INSERT_VALUES =
            "	values (msg.id,msg.st,msg.submit_time,msg.valid_time,msg.attempts, " +
                    "   msg.last_result,msg.last_try_time,msg.next_try_time,msg.oa,msg.da,  " +
                    "   msg.dda,msg.mr,msg.svc_type,msg.dr,msg.br,msg.src_msc,   " +
                    "   msg.src_imsi,msg.src_sme_n,msg.dst_msc,msg.dst_imsi,msg.dst_sme_n, " +
                    "   msg.route_id,msg.svc_id,msg.prty,msg.src_sme_id,msg.dst_sme_id, " +
                    "   msg.msg_ref,msg.seq_num,msg.arc,msg.body_len); " +
                    "end; ";

    protected Category logger = Category.getInstance(SmsExport.class);
    protected static String DS_SECTION = "";

    protected ExportSettings defaultExportSettings = null;

    protected Object generalLock = new Object();

    protected SmsExport(Config webConfig) throws AdminException {
        try {
            final String section = DS_SECTION;
            final String source = webConfig.getString(section + ".source");
            final String driver = webConfig.getString(section + ".driver");
            final String user = webConfig.getString(section + ".user");
            final String pass = webConfig.getString(section + ".pass");
            final String prefix = webConfig.getString(section + ".tables_prefix");
            defaultExportSettings = new ExportSettings(source, driver, user, pass, prefix);
            dateFormat.setTimeZone(TimeZone.getDefault());
        } catch (Exception e) {
            throw new AdminException("Failed to configure default export settings. Details: " + e.getMessage());
        }
    }

    public void export(ExportSettings export) throws AdminException {
        synchronized (generalLock) {
            _export((export != null) ? export : defaultExportSettings);
        }
    }

    public void export() throws AdminException {
        synchronized (generalLock) {
            _export(defaultExportSettings);
        }
    }

    protected abstract void _export(ExportSettings export) throws AdminException;

    protected ConnectionPool createDataSource(ExportSettings export) throws SQLException {
        Properties props = new Properties();
        props.setProperty("jdbc.source", export.getSource());
        props.setProperty("jdbc.driver", export.getDriver());
        props.setProperty("jdbc.user", export.getUser());
        props.setProperty("jdbc.pass", export.getPassword());
        props.setProperty("jdbc.min.connections", "0");
        props.setProperty("jdbc.max.idle.time", "240");
        props.setProperty("jdbc.pool.name", "smsexport");
        return new ConnectionPool(props);
    }

    protected void closeStatement(PreparedStatement stmt) {
        if (stmt != null)
            try {
                stmt.close();
            } catch (SQLException e) {
                logger.warn("Couldn't close statement");
            }
    }

    public void clearTable(Connection conn, String tablePrefix, Date datePrefix) throws SQLException {
        PreparedStatement clearStmt = null;
        if (conn instanceof oracle.jdbc.OracleConnection && datePrefix == null)
            clearStmt = conn.prepareStatement(DELETE_OP_SQL + tablePrefix);
        else
            clearStmt = conn.prepareStatement("DELETE FROM " + tablePrefix +
                    ((datePrefix != null) ? " WHERE LAST_TRY_TIME = '" + dateFormat.format(datePrefix) + "'" : ""));
        clearStmt.executeUpdate();
        conn.commit();
    }

    public ExportSettings getDefaultExportSettings() {
        return defaultExportSettings;
    }

}
