package storage;

import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.Date;


public class ResultSet {
    private java.sql.ResultSet sqlResult;

    public ResultSet(java.sql.ResultSet resultSet) throws StorageException{
        if(resultSet==null) {
            throw new StorageException("Argument is null", StorageException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        this.sqlResult = resultSet;
    }
    public boolean next() throws StorageException {
        try {
            return sqlResult.next();
        } catch (SQLException e) {
            throw new StorageException("Unable to move resultset cursor", StorageException.ErrorCode.ERROR_DB);
        }
    }
    public Subscription get() throws StorageException {
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
    public void close() throws StorageException {
        try {
            sqlResult.close();
        } catch (SQLException e) {
            throw new StorageException("Unable to close sql resultset ", e);
        }
    }
}
