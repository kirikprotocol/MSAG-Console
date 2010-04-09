package mobi.eyeline.welcomesms.backend;

import org.apache.log4j.Logger;

import java.io.*;
import java.util.Map;
import java.util.HashMap;
import java.util.TreeMap;

/**
 * author: alkhal
 */
public class LocationService {

  private Map countries = new HashMap(300);

  private final File file;

  private static final Logger logger = Logger.getLogger(LocationService.class);

  LocationService(File file) throws IOException {
    if(file == null || !file.exists()) {
      throw new FileNotFoundException("File is not found: "+(file == null ? null : file.getAbsolutePath()));
    }
    this.file = file;
    reload();
  }

  /** @noinspection EmptyCatchBlock*/
  public void reload() throws IOException{
    BufferedReader reader = null;
    Map tmp = new HashMap(300);
    try{
      reader = new BufferedReader(new FileReader(file));
      String line;
      while((line = reader.readLine()) != null) {
          tmp.put(line.substring(0, line.indexOf(",")).replaceAll("\\,",","), line.substring(line.indexOf(",")+1).replaceAll("\\,",","));
          logger.error("Can't parse country from line: "+line);
      }
      countries = tmp;
    }finally{
      if(reader != null) {
        try {
          reader.close();
        }catch(IOException e){}
      }
    }
  }

  public String getCountryById(String countryId)  {
    return (String)countries.get(countryId);
  }

  public Map getCountries() {
    return new TreeMap(countries);
  }
}
