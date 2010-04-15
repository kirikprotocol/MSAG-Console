package ru.novosoft.smsc.infosme.backend;

import org.apache.log4j.Category;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Properties;

/**
 * author: alkhal
 * @noinspection EmptyCatchBlock
 */
public class InfoSmeLicense {

  private static final Category category = Category.getInstance(InfoSmeLicense.class);

  private final Properties properties = new Properties();

  private static InfoSmeLicense instance;

  private final File file;

  private String organization;
  private String hostids;
  private int maxSmsThroughput;
  private Date licenseExpirationDate;
  private String licenseType;
  private String product;


  InfoSmeLicense(File f) throws IOException, ParseException {
    if(f == null || !f.exists()) {
      throw new IllegalArgumentException("License is not found: "+(f == null ? null : f.getAbsolutePath()));
    }
    this.file = f;
    InputStream is = null;
    try{
      is = new FileInputStream(f);
      properties.load(is);
    } finally {
      if(is != null) {
        try{
          is.close();
        }catch (IOException e){}
      }
    }
    this.organization = properties.getProperty("Organization");
    this.hostids = properties.getProperty("Hostids");
    this.maxSmsThroughput = Integer.parseInt(properties.getProperty("MaxSmsThroughput"));
    this.licenseExpirationDate = new SimpleDateFormat("yyyy-MM-dd").parse(properties.getProperty("LicenseExpirationDate"));
    this.licenseType = properties.getProperty("LicenseType");
    this.product = properties.getProperty("Product");
  }

  public static InfoSmeLicense getInstance() {
    return instance;
  }

  public File getFile() {
    return file;
  }

  public String getOrganization() {
    return organization;
  }

  public String getHostids() {
    return hostids;
  }

  public int getMaxSmsThroughput() {
    return maxSmsThroughput;
  }

  public String getLicenseType() {
    return licenseType;
  }

  public String getProduct() {
    return product;
  }

  public Date getLicenseExpirationDate() {
    return (Date)licenseExpirationDate.clone();
  }
}
