package mobi.eyeline.smsquiz.distribution;

import java.io.FileNotFoundException;
import java.io.IOException;

public interface ResultSet {
 
	public boolean next() throws IOException;
	public Object get();
	public void close();
}
 
