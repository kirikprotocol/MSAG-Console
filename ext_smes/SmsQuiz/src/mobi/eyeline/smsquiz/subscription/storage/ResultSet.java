package mobi.eyeline.smsquiz.subscription.storage;


public interface ResultSet {
    public boolean next() throws StorageException ;
    public Object get() throws StorageException ;
    public void close();
}
