package storage;

import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.Date;


public interface ResultSet {
    public boolean next() throws StorageException ;
    public Object get() throws StorageException ;
    public void close();
}
