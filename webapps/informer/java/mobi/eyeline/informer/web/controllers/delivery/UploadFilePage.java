package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.UploadController;
import org.apache.myfaces.trinidad.model.UploadedFile;

import javax.faces.context.FacesContext;
import java.io.*;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class UploadFilePage extends UploadController implements CreateDeliveryPage {

  private int current;

  private int maximum = Integer.MAX_VALUE;

  private Map<String, File> tmpFiles;
  private Map<String, RegionInfo> regions;

  private int abonentsSize;

  private final FileSystem fs;

  private int fileContentType = 1;
  private String fileEncoding;
  private boolean splitByRegions;

  private Configuration config;
  private File rejectedAddressesFile;
  private int rejectedAddressesCount;

  public UploadFilePage(Configuration config, String user) {
    this.config = config;
    fs = config.getFileSystem();
    tmpFiles = new HashMap<String, File>();
    regions = new HashMap<String, RegionInfo>();

    rejectedAddressesFile = new File(config.getWorkDir(), "rejected_addresses_" + user + System.currentTimeMillis());
    rejectedAddressesFile.deleteOnExit();

    fileEncoding = "windows-1251";
  }

  public CreateDeliveryPage process(String user, Configuration config, Locale locale) throws AdminException {
    if (isStoped() || getError() != null) {
      return new UploadFilePage(config, user);
    }
    next();

    DeliveryBuilder fact;
    if (!splitByRegions)
      fact= new SingleDeliveryBuilder(new ArrayList<File>(tmpFiles.values()).get(0), fileContentType == 1, config.getUser(user), config);
    else
      fact = new MultiDeliveryBuilder(tmpFiles, fileContentType == 1, config.getUser(user), config);

    if (rejectedAddressesFile != null) {
      try {
        config.getFileSystem().delete(rejectedAddressesFile);
      } catch (AdminException e) {
        logger.error(e,e);
      }
    }

    return new DeliveryEditPage(fact, fileContentType == 1, config, user);
  }

  public String getPageId() {
    return "DELIVERY_UPLOAD_FILE";
  }

  private void removeTmpFiles() {
    for (File file : tmpFiles.values()) {
      try {
        config.getFileSystem().delete(file);
      } catch (AdminException e) {
        logger.error(e,e);
      }
    }
    if (rejectedAddressesFile != null) {
      try {
        config.getFileSystem().delete(rejectedAddressesFile);
      } catch (AdminException e) {
        logger.error(e);
      }
    }
  }

  @SuppressWarnings({"ResultOfMethodCallIgnored"})
  public void cancel() {
    removeTmpFiles();
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

  public boolean isSplitByRegions() {
    return splitByRegions;
  }

  public void setSplitByRegions(boolean splitByRegions) {
    this.splitByRegions = splitByRegions;
  }

  public int getFileContentType() {
    return fileContentType;
  }

  public void setFileContentType(int content) {
    this.fileContentType = content;
  }

  public boolean isStoped() {
    return super.isStoped();
  }

  public Collection<RegionInfo> getRegions() {
    return new ArrayList<RegionInfo>(regions.values());
  }

  public void downloadRejectedLines(FacesContext context, OutputStream out) throws IOException {
    try {
      if (!getConfig().getFileSystem().exists(rejectedAddressesFile))
        return;
    } catch (AdminException e) {
      logger.error(e,e);
      return;
    }

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

  private Address getAddressFromLine(String line, int lineNumber) throws DeliveryControllerException {
    String addr;
    if (fileContentType == 1)
      addr = line.trim();
    else {
      int i = line.indexOf('|');
      if (i < 0)
        throw new DeliveryControllerException("invalid.string.format", String.valueOf(lineNumber), line);

      addr = line.substring(0, i);
      if (addr.length() >0 && addr.charAt(0) != '+') {
        if (addr.charAt(0) == '7')
          addr = '+' + addr;
        else if (addr.charAt(0) == '8')
          addr = "+7" + addr.substring(1);
      }
    }

    try {
      return new Address(addr);
    } catch (IllegalArgumentException e) {
      throw new DeliveryControllerException("invalid.msisdn", String.valueOf(lineNumber), addr);
    }
  }

  private boolean isRegionAllowed(Region r, User _user) {
    return _user.isAllRegionsAllowed() || (r != null && _user.getRegions() != null && _user.getRegions().contains(r.getRegionId()));
  }

  private void loadFile(UploadedFile file, String user) throws Exception {
    User _user = config.getUser(user);
    maximum = (int) file.getLength();
    BufferedReader is = null;
    PrintWriter rejectedAddresses = null;

    Map<Integer, PrintWriter> outputs = new HashMap<Integer, PrintWriter>();

    try {
      is = new BufferedReader(new InputStreamReader(file.getInputStream(), fileEncoding));
      rejectedAddresses = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(rejectedAddressesFile, false))));

      int lineNumber = 0;
      String line;
      while ((line = is.readLine()) != null && !isStoped()) {
        lineNumber++;
        current += line.length();

        if (line.trim().length() == 0)
          continue;

        // Достаем и проверяем адрес получателя
        Address address = getAddressFromLine(line, lineNumber);

        abonentsSize++;

        // Определяем регион и проверяем разрешен ли он пользователю
        Region r = config.getRegion(address);
        if (!isRegionAllowed(r, _user)) {
          rejectedAddresses.println(line);
          rejectedAddressesCount++;
          continue;
        }

        // Определяем имя региона и его идентификатор
        String regionName;
        int rId;
        if (r == null) {
          rId = 0;
          regionName = "Default";
        } else {
          rId = splitByRegions ? r.getRegionId() : 0;
          regionName = r.getName();
        }

        // Определяем файл, в который надо записать текущую строку
        PrintWriter os = outputs.get(rId);
        if (os == null) {
          File tmpFile = new File(config.getWorkDir(), "messages_" + rId + "_" + user + System.currentTimeMillis());
          tmpFiles.put(regionName, tmpFile);
          os = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(tmpFile, false))));
          outputs.put(rId, os);
        }

        RegionInfo rcounter = regions.get(regionName);
        if (rcounter == null) {
          rcounter = new RegionInfo(regionName);
          regions.put(regionName, rcounter);
        }

        rcounter.incAbonentsNumber();

        os.println(line);
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
      for (PrintWriter os : outputs.values()) {
        os.close();
      }
      if (rejectedAddresses != null)
        rejectedAddresses.close();
    }
  }

  @Override
  protected void _process(UploadedFile file, String user, Map<String, String> requestParams) throws Exception {
    try {
      loadFile(file, user);
      if (isStoped())
        removeTmpFiles();
    } catch (Exception e) {
      removeTmpFiles();
      throw e;
    }
  }

  public class RegionInfo {
    private String name;
    private int abonentsNumber;

    public RegionInfo(String name) {
      this.name = name;
    }

    public String getName() {
      return name;
    }

    public int getAbonentsNumber() {
      return abonentsNumber;
    }

    public void incAbonentsNumber() {
      abonentsNumber++;
    }
  }
}