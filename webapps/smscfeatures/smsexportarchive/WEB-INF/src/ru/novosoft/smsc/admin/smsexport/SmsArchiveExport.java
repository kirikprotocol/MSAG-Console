package ru.novosoft.smsc.admin.smsexport;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.smsstat.ExportSettings;
import ru.novosoft.smsc.admin.smsview.SmsQuery;
import ru.novosoft.smsc.admin.smsview.SmsRow;
import ru.novosoft.smsc.admin.smsview.SmsSet;
import ru.novosoft.smsc.admin.smsview.archive.SmsArchiveSource;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;
import ru.sibinco.util.conpool.ConnectionPool;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;

public class SmsArchiveExport extends SmsExport {
    protected static String DS_ARCH_SECTION = "archsave_datasource";

    protected final static String INSERT_VALUES =
            "	values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,   " +
                    "   ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    protected SMSCAppContext appContext = null;
    protected SmsArchiveSource archive = new SmsArchiveSource();
    protected Date date = null;

    protected static Object instanceLock = new Object();
    protected static SmsExport instance = null;

    public static SmsExport getInstance(Config webConfig, SMSCAppContext appContext) throws AdminException {
        DS_SECTION = DS_ARCH_SECTION;
        synchronized (instanceLock) {
            if (instance == null) instance = new SmsArchiveExport(webConfig, appContext);
            return (SmsExport) instance;
        }
    }

    protected SmsArchiveExport(Config webConfig, SMSCAppContext appContext) throws AdminException {
        super(webConfig);
        this.appContext = appContext;
        archive.init(appContext);
    }

    synchronized protected void _export(ExportSettings export) throws AdminException {
        if ((!appContext.getHostsManager().getServiceInfo(Constants.ARCHIVE_DAEMON_SVC_ID).isOnline())) {
            throw new AdminException("Archive Daemon is not running. ");
        }

        ConnectionPool ds = null;
        Connection conn = null;
        PreparedStatement insertStmt = null;
        try {
            long tm = 0;
            ds = createDataSource(export);
            conn = ds.getConnection();
            conn.setAutoCommit(false);
            int inserted = 0;
            if (conn == null) throw new SQLException("Failed to obtain connection");
            String tablePrefix = export.getTablesPrefix();
            String INSERT_SQL = INSERT_OP_SQL + tablePrefix + INSERT_FIELDS + INSERT_VALUES;
            insertStmt = conn.prepareStatement(INSERT_SQL);

            System.out.println("start clearing old data from Table in: " + new Date());
            tm = System.currentTimeMillis();
            clearTable(conn, tablePrefix, date);
            System.out.println("end clearing old data from Table in: " + new Date() + " spent: " + (System.currentTimeMillis() - tm) / 1000);

            SmsQuery query = new SmsQuery();
            query.setRowsMaximum(300000);
            SmsSet set = null;
            Calendar cal = new GregorianCalendar();
            Date fromDate, tillDate;
            cal.setTime(date);
            cal.set(Calendar.AM_PM, 0);
            cal.set(Calendar.HOUR, 0);
            cal.set(Calendar.MINUTE, 0);
            cal.set(Calendar.SECOND, 0);
            cal.set(Calendar.MILLISECOND, 0);
            for (int i = 0; i < 24; i++) {
                if (i > 11) {
                    cal.set(Calendar.AM_PM, 1);
                    cal.set(Calendar.HOUR, i - 12);
                } else
                    cal.set(Calendar.HOUR, i);
                inserted = 0;
                for (int j = 0; j < 60; j += 5) {
                    cal.set(Calendar.MINUTE, j);
                    cal.set(Calendar.SECOND, 0);
                    cal.set(Calendar.MILLISECOND, 0);
                    fromDate = cal.getTime();
                    cal.set(Calendar.MINUTE, j + 4);
                    cal.set(Calendar.SECOND, 59);
                    cal.set(Calendar.MILLISECOND, 999);
                    tillDate = cal.getTime();
                    query.setFromDate(fromDate);
                    query.setFilterFromDate(true);
                    query.setTillDate(tillDate);
                    query.setFilterTillDate(true);
                    set = archive.getSmsSet(query);
                    for (int k = 0; k < set.getRowsCount(); k++) {
                        SmsRow row = set.getRow(k);
                        setValues(insertStmt, row);
                        inserted += insertStmt.executeUpdate();
                        //insertStmt.clearParameters();
                    }
                }
                if (inserted > 0) conn.commit();
            }
        }
        catch (java.sql.SQLException e) {
            e.printStackTrace();
            throw new AdminException(e.getMessage());
        }
        finally {
            closeStatement(insertStmt);
            try {
                if (conn != null) conn.close();
            }
            catch (Throwable th) {
                logger.error("Failed to close connection", th);
            }
            try {
                if (ds != null) ds.shutdown();
            }
            catch (Throwable th) {
                logger.error("Failed to close data source", th);
            }
        }
    }

    private void setValues(PreparedStatement stmt, SmsRow row) throws SQLException {
        int pos = 1;
        stmt.setLong(pos++, row.getId());
        stmt.setInt(pos++, row.getStatusInt());
        if (row.getSubmitTime() != null)
            stmt.setDate(pos++, new java.sql.Date(row.getSubmitTime().getTime()));
        else
            stmt.setDate(pos++, null);
        if (row.getValidTime() != null)
            stmt.setDate(pos++, new java.sql.Date(row.getValidTime().getTime()));
        else
            stmt.setDate(pos++, null);
        stmt.setInt(pos++, row.getAttempts());
        stmt.setInt(pos++, row.getLastResult());
        if (row.getLastTryTime() != null)
            stmt.setDate(pos++, new java.sql.Date(row.getLastTryTime().getTime()));
        else
            stmt.setDate(pos++, null);
        if (row.getNextTryTime() != null)
            stmt.setDate(pos++, new java.sql.Date(row.getNextTryTime().getTime()));
        else
            stmt.setDate(pos++, null);
        if (row.getOriginatingAddressMask() != null)
            stmt.setString(pos++, row.getOriginatingAddressMask().getMask());
        else
            stmt.setString(pos++, null);
        if (row.getDestinationAddressMask() != null)
            stmt.setString(pos++, row.getDestinationAddressMask().getMask());
        else
            stmt.setString(pos++, null);
        if (row.getDealiasedDestinationAddressMask() != null)
            stmt.setString(pos++, row.getDealiasedDestinationAddressMask().getMask());
        else
            stmt.setString(pos++, null);
        stmt.setInt(pos++, row.getMessageReference());
        stmt.setString(pos++, row.getServiceType());
        stmt.setShort(pos++, row.getDeliveryReport());
        stmt.setShort(pos++, row.getBillingRecord());
        if (row.getOriginatingDescriptor() != null) {
            stmt.setString(pos++, row.getOriginatingDescriptor().getMsc());
            stmt.setString(pos++, row.getOriginatingDescriptor().getImsi());
            stmt.setLong(pos++, row.getOriginatingDescriptor().getSme());
        } else {
            stmt.setString(pos++, null);
            stmt.setString(pos++, null);
            stmt.setLong(pos++, 0);
        }
        if (row.getDestinationDescriptor() != null) {
            stmt.setString(pos++, row.getDestinationDescriptor().getMsc());
            stmt.setString(pos++, row.getDestinationDescriptor().getImsi());
            stmt.setLong(pos++, row.getDestinationDescriptor().getSme());
        } else {
            stmt.setString(pos++, null);
            stmt.setString(pos++, null);
            stmt.setLong(pos++, 0);
        }
        stmt.setString(pos++, row.getRouteId());
        stmt.setLong(pos++, row.getServiceId());
        stmt.setLong(pos++, row.getPriority());
        stmt.setString(pos++, row.getSrcSmeId());
        stmt.setString(pos++, row.getDstSmeId());
        stmt.setShort(pos++, row.getConcatMsgRef());
        stmt.setShort(pos++, row.getConcatSeqNum());
        stmt.setByte(pos++, row.getArc());
        stmt.setInt(pos++, row.getBodyLen());
    }

    public void setDate(Date date) {
        this.date = date;
    }
}
