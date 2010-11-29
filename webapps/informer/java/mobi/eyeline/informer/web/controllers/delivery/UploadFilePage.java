package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryPrototype;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.UploadController;
import org.apache.myfaces.trinidad.model.UploadedFile;

import javax.faces.context.FacesContext;
import java.io.*;
import java.util.Date;
import java.util.Locale;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
public class UploadFilePage extends UploadController implements CreateDeliveryPage {

  private int current;

  private int maximum = Integer.MAX_VALUE;

  private DeliveryPrototype delivery;

  private File tmpFile;

  private int abonentsSize;

  private final FileSystem fs;

  private int fileContentType = 1;
  private String fileEncoding;

  private Configuration config;
  private String user;
  private File rejectedAddressesFile;
  private int rejectedAddressesCount;

  public UploadFilePage(Configuration config, String user) {
    this.config = config;
    this.user = user;
    fs = config.getFileSystem();
    tmpFile = new File(config.getWorkDir(), "messages_" + user + System.currentTimeMillis());
    rejectedAddressesFile = new File(config.getWorkDir(), "rejected_addresses_" + user + System.currentTimeMillis());
    rejectedAddressesFile.deleteOnExit();

    fileEncoding = "windows-1251";
  }

  public CreateDeliveryPage process(String user, Configuration config, Locale locale) throws AdminException {
    if (isStoped() || getError() != null) {
      return new UploadFilePage(config, user);
    }
    next();
    return new DeliveryEditPage(delivery, fileContentType == 1, tmpFile, config);
  }

  public int getFileContentType() {
    return fileContentType;
  }

  public void setFileContentType(int content) {
    this.fileContentType = content;
  }

  @Override
  public String upload() {
    delivery = new DeliveryPrototype();
    try {
      config.copyUserSettingsToDeliveryPrototype(user, delivery);
      delivery.setStartDate(new Date());
    } catch (AdminException e) {
      addError(e);
      return null;
    }

    return super.upload();
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

  public int getAbonentsSize() {
    return abonentsSize;
  }

  public boolean isStoped() {
    return super.isStoped();
  }

  private String getAddressFromLine(String line, int lineNumber, boolean containsText) throws DeliveryControllerException {
    if (!containsText)
      return line.trim();

    int i = line.indexOf('|');
    if (i < 0)
      throw new DeliveryControllerException("invalid.string.format", String.valueOf(lineNumber), line);
    return line.substring(0, i);
  }

  private boolean isAddressAllowed(Address address, User u) {
    if (u.isAllRegionsAllowed())
      return true;

    Region r = config.getRegion(address);
    return r != null && u.getRegions() != null && u.getRegions().contains(r.getRegionId());
  }

  public String getRejectedAddressesFile() {
    return rejectedAddressesFile.getAbsolutePath();
  }

  public int getRejectedAddressesCount() {
    return rejectedAddressesCount;
  }

  public String getFileEncoding() {
    return fileEncoding;
  }

  public void setFileEncoding(String fileEncoding) {
    this.fileEncoding = fileEncoding;
  }

  public void downloadRejectedLines(FacesContext context, OutputStream out) throws IOException {
    if (!rejectedAddressesFile.exists())
      return;

    PrintWriter w = new PrintWriter(new OutputStreamWriter(out, fileEncoding));
    BufferedReader r = null;
    try {
      try {
        r = new BufferedReader(new InputStreamReader(fs.getInputStream(rejectedAddressesFile)));
      } catch (AdminException e) {
        logger.error(e, e);
        throw new IOException(e.getMessage());
      }
      String line;
      while ((line = r.readLine()) != null)
        w.println(line);

    } finally {
      if (r != null) {
        try {
          r.close();
        } catch (IOException ignored) {
        }
      }
    }
    w.flush();
  }

  @Override
  protected void _process(UploadedFile file, String user, Map<String, String> requestParams) throws Exception {
    User _user = config.getUser(user);
    maximum = (int) file.getLength();
    BufferedReader is = null;
    PrintWriter os = null;
    PrintWriter rejectedAddresses = null;
    try {
      is = new BufferedReader(new InputStreamReader(file.getInputStream(), fileEncoding));
      os = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(tmpFile, false))));
      rejectedAddresses = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(rejectedAddressesFile, false))));

      int lineNumber = 0;
      String line;
      while ((line = is.readLine()) != null && !isStoped()) {
        lineNumber++;
        current += line.length();

        if (line.trim().length() == 0)
          continue;

        String addressStr = getAddressFromLine(line, lineNumber, fileContentType != 1);
        Address address;
        try {
          address = new Address(addressStr);
        } catch (IllegalArgumentException e) {
          throw new DeliveryControllerException("invalid.msisdn", String.valueOf(lineNumber), addressStr);
        }

        if (!isAddressAllowed(address, _user)) {
          rejectedAddresses.println(line);
          rejectedAddressesCount++;
        } else
          os.println(line);

        abonentsSize++;
      }

      if (!isStoped())
        current = maximum;

      if (rejectedAddressesCount == abonentsSize)
        throw new DeliveryControllerException("all.msisdns.rejected");

    } finally {
      if (is != null) {
        try {
          is.close();
        } catch (IOException ignored) {
        }
      }
      if (os != null) {
        os.close();
      }
      if (rejectedAddresses != null)
        rejectedAddresses.close();
    }
  }
}