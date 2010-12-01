package mobi.eyeline.informer.web.controllers.blacklist;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.controllers.UploadController;
import org.apache.myfaces.trinidad.model.UploadedFile;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * Контроллер для загрузки запрещённых номеров
 *
 * @author Aleksandr Khalitov
 */

@SuppressWarnings({"EmptyCatchBlock"})
public class BlacklistUploadController extends UploadController {

  private int unrecognized;

  private int uploaded;

  private int current;

  private int maximum = Integer.MAX_VALUE;    


  public boolean isBlacklistEnabled() {
    try{
      return getConfig().isBlackListEnabled();
    }catch (AdminException e) {
      addError(e);
      return false;
    }
  }

  public BlacklistUploadController() {
  }

  public int getUnrecognized() {
    return unrecognized;
  }

  public int getUploaded() {
    return uploaded;
  }


  public int getCurrent() {
    return current;
  }

  public int getMaximum() {
    return maximum;
  }

  @Override
  protected String _next() {
    unrecognized = 0;
    uploaded = 0;
    current = 0;
    maximum = Integer.MAX_VALUE;
    return "BLACKLIST";
  }

  @Override
  protected void _process(UploadedFile file, String user, Map<String, String> requestParams) throws Exception {
    maximum = (int) file.getLength();
    boolean add = requestParams.get("file_add") != null;
    String line;
    List<String> list = new ArrayList<String>(1000);
    BufferedReader reader = null;
    try {
      reader = new BufferedReader(new InputStreamReader(file.getInputStream()));
      while ((line = reader.readLine()) != null && !isStoped()) {
        line = line.trim();
        current += line.length();
        if (!Address.validate(line)) {
          unrecognized++;
          continue;
        }
        list.add(line);
        if (list.size() == 1000) {
          if (add) {
            getConfig().addInBlacklist(list, user);
          } else {
            getConfig().removeFromBlacklist(list, user);
          }
          uploaded += 1000;
          list.clear();
        }
      }
      if (!list.isEmpty() && !isStoped()) {
        int s = list.size();
        if (add) {
          getConfig().addInBlacklist(list, user);
        } else {
          getConfig().removeFromBlacklist(list, user);
        }
        uploaded += s;
      }
      if (!isStoped()) {
        current = maximum;
      }
    } finally {
      if (reader != null) {
        try {
          reader.close();
        } catch (IOException e) {
        }
      }
    }
  }

}
