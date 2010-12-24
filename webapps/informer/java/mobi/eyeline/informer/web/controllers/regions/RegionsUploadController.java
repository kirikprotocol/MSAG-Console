package mobi.eyeline.informer.web.controllers.regions;

import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.WebContext;
import mobi.eyeline.informer.web.config.InformerTimezone;
import mobi.eyeline.informer.web.controllers.UploadController;
import org.apache.myfaces.trinidad.model.UploadedFile;

import javax.faces.model.SelectItem;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.util.*;

/**
 * Контроллер для загрузки файла с регионами
 *
 * @author Aleksandr Khalitov
 */
public class RegionsUploadController extends UploadController {

  private int uploaded;

  private int current;

  private int maximum = Integer.MAX_VALUE;



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
    uploaded = 0;
    current = 0;
    maximum = Integer.MAX_VALUE;
    return "REGION";
  }

  private static String readProperty(StringTokenizer t) {
    String tmp = t.nextToken().trim();
    while (tmp.endsWith("\\")) {
      tmp = tmp.substring(0, tmp.length() - 1);
      tmp += t.nextToken();
    }
    return tmp;
  }


  private Region readRegion(String line) throws Exception {
    Region r = new Region();
    StringTokenizer t = new StringTokenizer(line, ",");
    try {
      r.setName(readProperty(t));
      r.setSmsc(readProperty(t));
      r.setMaxSmsPerSecond(Integer.parseInt(readProperty(t)));
      String alias = readProperty(t);
      InformerTimezone tz = WebContext.getInstance().getWebTimezones().getTimezoneByAlias(alias);
      if (tz == null)
        throw new IllegalArgumentException(alias);
      r.setTimeZone(tz.getTimezone());
    } catch (NumberFormatException ex) {
      throw new IllegalArgumentException(ex);
    } catch (NoSuchElementException ex) {
      throw new IllegalArgumentException(ex);
    }
    return r;
  }

  private String charset;

  public String getCharset() {
    return charset;
  }

  public void setCharset(String charset) {
    this.charset = charset;
  }

  public List<SelectItem> getCharsets() {
    Locale l = getLocale();
    List<SelectItem> result = new ArrayList<SelectItem>(3);
    Charset c = Charset.forName("windows-1251");
    result.add(new SelectItem(c.name(), c.displayName(l)));
    c = Charset.forName("utf-8");
    result.add(new SelectItem(c.name(), c.displayName(l)));
    c = Charset.forName("koi8-r");
    result.add(new SelectItem(c.name(), c.displayName(l)));
    return result;
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  @Override
  protected void _process(UploadedFile file, String user, Map<String, String> requestParams) throws Exception {

    maximum = 2;
    current = 0;

    List<Region> regions = new ArrayList<Region>();

    BufferedReader is = null;
    try {
      is = new BufferedReader(new InputStreamReader(file.getInputStream(), Charset.forName(charset)));
      String line;
      Region r = null;
      while ((line = is.readLine()) != null && !isStoped()) {
        line = line.trim();
        if (line.indexOf(',') != -1) {
          r = readRegion(line);
          regions.add(r);
        } else {
          if (r == null) {
            throw new IllegalArgumentException("Illegal file format");
          }
          if (!Address.validate(line)) {
            throw new IllegalArgumentException("Illegal mask: " + line);
          }
          r.addMask(new Address(line));

        }
      }

      current = 1;

      if (!isStoped())
        getConfig().updateRegionsConfiguration(regions);

      current = maximum;

    } finally {
      if (is != null) {
        try {
          is.close();
        } catch (IOException e) {
        }
      }
    }

  }
}
