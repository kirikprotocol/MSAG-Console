package mobi.eyeline.smsquiz.distribution;

import java.io.FileNotFoundException;
import java.io.IOException;

public interface ResultSet {
 
	public boolean next();
	public Object get();
	public void close();
}
 
