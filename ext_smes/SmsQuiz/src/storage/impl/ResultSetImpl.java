package storage.impl;

import storage.ResultSet;
import storage.StorageException;

import java.sql.Connection;
import java.sql.PreparedStatement;

/**
 * Created by IntelliJ IDEA.
 * User: alkhal
 * Date: 30.09.2008
 * Time: 14:49:07
 * To change this template use File | Settings | File Templates.
 */
public class ResultSetImpl {
    public static ResultSet getInstance(java.sql.ResultSet resultSet, Connection connection, PreparedStatement prepStatement) throws StorageException {
        return new SubscriptionResultSet(resultSet,connection, prepStatement);
    }
}
