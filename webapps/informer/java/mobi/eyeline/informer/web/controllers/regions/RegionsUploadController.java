package mobi.eyeline.informer.web.controllers.regions;

import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.regions.RegionException;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.controllers.UploadController;
import org.apache.myfaces.trinidad.model.UploadedFile;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.*;

/**
 * Контроллер для загрузки файла с регионами
 * @author Aleksandr Khalitov
 */
public class RegionsUploadController extends UploadController{

  private int uploaded;

  private int current;

  private int maximum = Integer.MAX_VALUE;

  private Map<String, Region> regions = new HashMap<String, Region>();


  public int getUploaded() {
    return uploaded;
  }

  @Override
  public int getCurrent() {
    return current;
  }

  @Override
  public int getMaximum() {
    return maximum;
  }

  @Override
  protected String _next() {
    regions.clear();
    uploaded = 0;
    current = 0;
    maximum = Integer.MAX_VALUE;
    return "REGION";
  }

  private static String readProperty(StringTokenizer t) {
    String tmp = t.nextToken().trim();
    while(tmp.endsWith("\\")) {
      tmp = tmp.substring(0,tmp.length()-1);
      tmp+=t.nextToken();
    }
    return tmp;
  }


  private static Region readRegion(String line) throws Exception{
    Region r = new Region();
    StringTokenizer t = new StringTokenizer(line,",");
    try{
      r.setName(readProperty(t));
      r.setSmsc(readProperty(t));
      r.setMaxSmsPerSecond(Integer.parseInt(readProperty(t)));
      r.setTimeZone(TimeZone.getTimeZone(readProperty(t)));
    }catch (NumberFormatException ex){
      throw new IllegalArgumentException(ex);
    }catch (NoSuchElementException ex) {
      throw new IllegalArgumentException(ex);
    }
    return r;
  }

  @Override
  protected void _process(UploadedFile file, String user, Map<String, String> requestParams) throws Exception {

    maximum = 3*((int)file.getLength());
    BufferedReader is = null;
    try{
      is = new BufferedReader(new InputStreamReader(file.getInputStream()));
      String line;
      Region r = null;
      while((line = is.readLine()) != null && !isStoped()) {
        line = line.trim();
        if(line.indexOf(',') != -1) {
          r = readRegion(line);
          if(getConfig().getSmsc(r.getSmsc()) == null) {
            throw new RegionException("smsc_not_exist", r.getSmsc());
          }
        }else {
          if(r == null) {
            throw new IllegalArgumentException("Illegal file format");
          }
          if(!Address.validate(line)) {
            throw new IllegalArgumentException("Illegal mask: "+line);
          }
          r.addMask(new Address(line));

          if(regions.containsKey(line)) {
            throw new RegionException("masks_intersection", line);
          }
          regions.put(line,r);
        }
        current+=line.length();
      }
      Collection<Region> oldRegions = getConfig().getRegions();
      for(Region region : oldRegions) {
        if(!isStoped()) {
          getConfig().removeRegion(region.getRegionId(), user);
          current+=maximum/3/oldRegions.size();
        }else {
          break;
        }
      }

      Set<Region> newRegions = new HashSet<Region>(regions.values());
      for(Region region : newRegions) {
        if(!isStoped()) {
          getConfig().addRegion(region, user);
          current+=maximum/3/newRegions.size();
          uploaded++;
        }else {
          break;
        }
      }
      if(!isStoped()) {
        current = maximum;
      }

    }finally {
      if(is != null) {
        try{
          is.close();
        }catch (IOException e){}
      }
    }

  }
}
