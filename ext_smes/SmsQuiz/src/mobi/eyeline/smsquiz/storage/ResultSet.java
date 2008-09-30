package mobi.eyeline.smsquiz.storage;


public interface ResultSet {
    public boolean next() throws StorageException ;
    public Object get() throws StorageException ;
    public void close();
}
