package mobi.eyeline.smsquiz.storage.impl;

import mobi.eyeline.smsquiz.storage.StorageException;
import mobi.eyeline.smsquiz.storage.Subscription;
import mobi.eyeline.smsquiz.storage.ResultSet;

import java.sql.SQLException;
import java.sql.Timestamp;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.util.Date;

import org.apache.log4j.Logger;


public class SubscriptionResultSet implements ResultSet {

    private static Logger logger = Logger.getLogger(SubscriptionResultSet.class);
    private java.sql.ResultSet sqlResult;
    private Connection connection;
    private PreparedStatement prepStatement;

    public SubscriptionResultSet(java.sql.ResultSet resultSet, Connection connection, PreparedStatement prepStatement) throws StorageException {
        if(resultSet==null) {
            throw new StorageException("Argument is null", StorageException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        this.connection = connection;
        this.sqlResult = resultSet;
        this.prepStatement = prepStatement;
    }
    public boolean next() throws StorageException {
        try {
            return sqlResult.next();
        } catch (SQLException e) {
            throw new StorageException("Unable to move resultset cursor", e);
        }
    }
    public Object get() throws StorageException {
        Timestamp timestamp;
        Subscription subscription = new Subscription();
        try{
            subscription.setAddress(sqlResult.getString("address"));
            if((timestamp = sqlResult.getTimestamp("start_date"))!=null)
                subscription.setStartDate(new Date(timestamp.getTime()));
            if((timestamp = sqlResult.getTimestamp("end_date"))!=null)
                subscription.setEndDate(new Date(timestamp.getTime()));
        } catch (SQLException e) {
            throw new StorageException("Unable to get subscription from the dataBase ", e);
        }
        return subscription;
    }
    public void close(){
        try {
            if(sqlResult!=null)
                sqlResult.close();
        } catch (SQLException e) {
            logger.error("Unable to close sql resultset ", e);
        }
        try {
            if(prepStatement!=null)
                prepStatement.close();
        } catch (SQLException e) {
            logger.error("Unable to close sql resultset ", e);
        }
        try {
            if(connection!=null)
                connection.close();
        } catch (SQLException e) {
            logger.error("Unable to close connection", e);
        }
    }

}
