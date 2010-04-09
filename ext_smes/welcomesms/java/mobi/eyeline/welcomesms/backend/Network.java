package mobi.eyeline.welcomesms.backend;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.admin.AdminException;

import java.io.IOException;

/**
 * author: alkhal
 */
public class Network {

  private Integer id;

  private int mcc;

  private int mnc;

  private String name;

  private String countryId;

  public static final String NETWORKS_PREFIX = "networks";


  public Integer getId() {
    return id;
  }

  public void setId(int id) {
    this.id = new Integer(id);
  }

  public int getMcc() {
    return mcc;
  }

  public void setMcc(int mcc) {
    this.mcc = mcc;
  }

  public int getMnc() {
    return mnc;
  }

  public void setMnc(int mnc) {
    this.mnc = mnc;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public String getCountryId() {
    return countryId;
  }

  public void setCountryId(String countryId) {
    this.countryId = countryId;
  }

  static Network loadFromConfig(int id, Config c) throws Config.WrongParamTypeException, Config.ParamNotFoundException {
    Network n = new Network();
    n.id = new Integer(id);
    n.name = c.getString(new StringBuffer().append(NETWORKS_PREFIX).append('.').append(id).append(".name").toString());
    n.mcc = c.getInt(new StringBuffer().append(NETWORKS_PREFIX).append('.').append(id).append(".mcc").toString());
    n.mnc = c.getInt(new StringBuffer().append(NETWORKS_PREFIX).append('.').append(id).append(".mnc").toString());
    n.countryId = c.getString(new StringBuffer().append(NETWORKS_PREFIX).append('.').append(id).append(".countryId").toString());
    return n;
  }

  void save(Config c) throws AdminException, Config.WrongParamTypeException, IOException {
    c.setString(new StringBuffer().append(NETWORKS_PREFIX).append('.').append(id).append(".name").toString(), name);
    c.setInt(new StringBuffer().append(NETWORKS_PREFIX).append('.').append(id).append(".mcc").toString(),mcc);
    c.setInt(new StringBuffer().append(NETWORKS_PREFIX).append('.').append(id).append(".mnc").toString(),mnc);
    c.setString(new StringBuffer().append(NETWORKS_PREFIX).append('.').append(id).append(".countryId").toString(),countryId);
    c.save();
  }

}
