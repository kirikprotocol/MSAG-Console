package mobi.eyeline.smsquiz.distribution;

import java.io.*;
import java.util.Set;
import java.util.HashSet;

public class DistributionResultSet implements ResultSet {
 
    private File currentFile;
    private String strReturn;
    private Set<File> files;
    private BufferedReader reader;

    public DistributionResultSet(Set<File> files) {
        this.files = new HashSet<File>();
        for(File f:files) {
            this.files.add(f);
        }
    }

    public boolean next() throws IOException {
        String line = null;
        if(files.size()==0) {
            return false;
        }
        if(currentFile==null) {
            currentFile = files.iterator().next();
            reader = new BufferedReader(new FileReader(currentFile));

        }
        if((line=reader.readLine())!=null) {
            strReturn = line;
            return true;
        } else {
            reader.close();     //todo
            files.remove(currentFile);
            currentFile=null;
            return next();
        }

    }

	public Object get() {
		return strReturn;
	}

	public void close() {
	 
	}
	 
}
 
