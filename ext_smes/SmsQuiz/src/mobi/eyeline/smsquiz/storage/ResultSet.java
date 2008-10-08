package mobi.eyeline.smsquiz.storage;

import mobi.eyeline.smsquiz.storage.StorageException;


public interface ResultSet {
    public boolean next() throws StorageException;
    public Object get() throws StorageException ;
    public void close();
}
