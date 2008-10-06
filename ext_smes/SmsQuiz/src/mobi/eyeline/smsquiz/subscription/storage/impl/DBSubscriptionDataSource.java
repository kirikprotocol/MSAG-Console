package mobi.eyeline.smsquiz.subscription.storage.impl;

import mobi.eyeline.smsquiz.subscription.storage.*;
import mobi.eyeline.smsquiz.subscription.storage.ResultSet;

import java.util.Date;
import java.sql.*;
import java.io.InputStream;
import java.io.IOException;

import snaq.db.ConnectionPool;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;
import org.apache.log4j.Logger;


public class DBSubscriptionDataSource implements SubscriptionDataSource {
    
    private static Logger logger = Logger.getLogger(DBSubscriptionDataSource.class);
    private ConnectionPool pool;

    private String properties = "smsquiz.properties";
    private PropertiesConfig sql;

    public DBSubscriptionDataSource() throws StorageException {

        pool = ConnectionPoolFactory.createConnectionPool("smsquiz", Integer.MAX_VALUE, 60000);
        InputStream is = DBSubscriptionDataSource.class.getResourceAsStream(properties);
        sql = new PropertiesConfig();
        try {
            sql.load(is);        
        } catch (IOException e) {
            logger.error("Error load config properties",e);
            throw new StorageException("Error load config properties", e);
        } finally {
            try{
                if(is!=null)
                    is.close();
            } catch (IOException e1) {
                logger.error("Can't close stream",e1);
                throw new StorageException("Error load config properties", e1);
            }
        }
    }

    private String getSql(java.lang.String string) throws StorageException {
        try {
            return sql.getString(string);
        } catch (ConfigException e) {
            throw new StorageException("Can't find sql: "  + string, StorageException.ErrorCode.ERROR_CONFIG);
        }
    }

    public void save(Subscription subscription) throws StorageException {
         if (subscription.getAddress()==null){
             throw new StorageException("Wrong arguments due creating subscription", StorageException.ErrorCode.ERROR_WRONG_REQUEST);
         }
         if(subscription.getStartDate()==null) {
             subscription.setStartDate(new Date());
         }

         Connection connection = null;
         PreparedStatement prepStatement = null;

         try{
             connection = pool.getConnection();
             prepStatement = connection.prepareStatement(getSql("smsquiz.subscription.save"));
             prepStatement.setString(1, subscription.getAddress());
             prepStatement.setTimestamp(2, new Timestamp(subscription.getStartDate().getTime()));
             if(subscription.getEndDate()!=null) {
                prepStatement.setTimestamp(3, new Timestamp(subscription.getEndDate().getTime()));
             }
             else {
                prepStatement.setNull(3, java.sql.Types.TIMESTAMP);
             }
             prepStatement.executeUpdate();

             if (logger.isInfoEnabled()){
                 logger.info("Succesful DB insertion of subscription" + subscription);
             }
         }catch(SQLException exc){

             logger.error("Unable to insert subscription to the dataBase: " + subscription, exc);
             throw new StorageException("Unable to insert subscription to the dataBase: ", exc);

         }finally{
             closeConn(connection,prepStatement,null);
         }
    }

    public Subscription get(String address) throws StorageException {
        Subscription subscription = null;
        Connection connection = null;
        PreparedStatement prepStatement = null;
        java.sql.ResultSet sqlResult = null;

         try{
            connection = pool.getConnection();

            prepStatement = connection.prepareStatement(getSql("smsquiz.subscription.get.by.address"),java.sql.ResultSet.CONCUR_READ_ONLY);
            prepStatement.setFetchSize(Integer.MIN_VALUE);
            prepStatement.setString(1, address);

            sqlResult = prepStatement.executeQuery();

            if(sqlResult.next()) {
                subscription = new Subscription();
                subscription.setAddress(address);
                subscription.setStartDate(new Date(sqlResult.getTimestamp("start_date").getTime()));
                Timestamp timestamp;
                if((timestamp = sqlResult.getTimestamp("end_date"))!=null) {
                    subscription.setEndDate(new Date(timestamp.getTime()));
                }
            }

            if (logger.isInfoEnabled()){
                logger.info("Succesful get subscription" + subscription);
            }
         }catch(SQLException exc){
             logger.error("Unable to get subscription from the dataBase with address: " + address, exc);
             throw new StorageException("Unable to get subscription from the dataBase with address: " + address, exc);

         }finally{
             closeConn(connection,prepStatement,sqlResult);
         }
         return subscription;
    }

    public ResultSet list(Date date) throws StorageException{
        if(date==null) {
            throw new StorageException("Argument is null", StorageException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        Connection connection = null;
        PreparedStatement prepStatement = null;
        java.sql.ResultSet sqlResult = null;

        try{
            connection = pool.getConnection();

            prepStatement = connection.prepareStatement(getSql("smsquiz.subscription.get.by.date"), java.sql.ResultSet.CONCUR_READ_ONLY);
            prepStatement.setFetchSize(Integer.MIN_VALUE);
            Timestamp timestamp = new Timestamp(date.getTime());
            prepStatement.setTimestamp(1, timestamp );
            prepStatement.setTimestamp(2, timestamp );

            sqlResult = prepStatement.executeQuery();

            if (logger.isInfoEnabled()){
                logger.info("Succesful get list of subscription");
            }
         }catch(SQLException exc){
             logger.error("Unable to get list of subscriptions from the dataBase", exc);
             throw new StorageException("Unable to get list of subscriptions from the dataBase", exc);

         }
        return new SubscriptionResultSet(sqlResult,connection, prepStatement);
    }


    public void remove(String address) throws StorageException{
        if(address==null) {
            throw new StorageException("Argument is null", StorageException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        Connection connection = null;
        PreparedStatement prepStatement = null;
        java.sql.ResultSet sqlResult = null;

        try{
            connection = pool.getConnection();

            prepStatement = connection.prepareStatement(getSql("smsquiz.subscription.remove.by.address"));
            prepStatement.setString(1,address);
            prepStatement.execute();
            if (logger.isInfoEnabled()){
                logger.info("Succesful get subscribed info by address");
            }

         }catch(SQLException exc){
             logger.error("Unable to get subscribed info by address: "+address, exc);
             throw new StorageException("Unable get subscribed info by address: ", exc);

         }finally{
             closeConn(connection,prepStatement,sqlResult);
         }
    }

    public void close() {
        pool.close();
    }

    private void closeConn(Connection connection, PreparedStatement preparedStatement, java.sql.ResultSet resultSet) {
        try{
            if(connection!=null) {
                connection.close();
            }
        } catch (SQLException exc) {
            logger.error("Unable to close connection", exc);
        }
        try{
            if(preparedStatement!=null) {
                preparedStatement.close();
            }
        } catch (SQLException exc) {
            logger.error("Unable to close connection", exc);

        }
        try{
            if(resultSet!=null) {
                resultSet.close();
            }
        } catch (SQLException exc) {
            logger.error("Unable to close connection", exc);
        }
    }
}
