package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryException;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.UploadController;
import org.apache.myfaces.trinidad.model.UploadedFile;

import java.io.*;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
public class UploadFilePage extends UploadController implements CreateDeliveryPage{

  private int current;

  private int maximum = Integer.MAX_VALUE;

  private Delivery delivery;

  private File tmpFile;

  public UploadFilePage(Delivery delivery, File tmpFile) {
    this.delivery = delivery;
    this.tmpFile = tmpFile;
  }

  public CreateDeliveryPage process(String user, Configuration config) throws AdminException {
    if(isStoped() || getError() != null) {
      return new StartPage();
    }
    next();
    return new DeliveryEditPage(delivery, tmpFile);
  }

  public String getPageId() {
    return "DELIVERY_UPLOAD_FILE";
  }

  @SuppressWarnings({"ResultOfMethodCallIgnored"})
  public void cancel() {
    tmpFile.delete();
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
    return null;
  }

  @Override
  protected void _process(UploadedFile file, String user, Map<String, String> requestParams) throws Exception {
    maximum = (int)file.getLength();
    BufferedReader is = null;
    PrintWriter os = null;
    try{
      is = new BufferedReader(new InputStreamReader(file.getInputStream()));
      os = new PrintWriter(new BufferedWriter(new FileWriter(tmpFile)));
      String line;
      while((line = is.readLine()) != null && !isStoped()) {
        String address;
        if(delivery.getType() == Delivery.Type.SingleText) {
          address = line;
        }else {
          String[] s = line.split(",",2);
          if(s.length == 1) {
            throw new IllegalArgumentException("Illegal file");
          }
          address = s[0];
        }
        if(!Address.validate(address)) {
          throw new DeliveryException("illegal_address",address);
        }
        os.println(line);
        current+=line.length();
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
      if(os != null) {
        os.close();
      }
    }
  }
}
