package mobi.eyeline.welcomesms.backend;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.admin.AdminException;

import java.util.Collection;
import java.util.Map;
import java.util.HashMap;
import java.util.Iterator;
import java.io.RandomAccessFile;
import java.io.File;
import java.io.IOException;

import org.apache.log4j.Logger;

/**
 * author: alkhal
 */
class NetworksManager {

  private static final Logger logger = Logger.getLogger(NetworksManager.class);

  private final Config config;

  private RandomAccessFile idFile;

  private int id;

  private final Map networks = new HashMap();

  NetworksManager(String configDir, Config c) throws AdminException{
    if(c == null) {
      throw new AdminException("Config is null");
    }

    this.config = c;

    try{
      initIdFile(configDir);
    }catch(IOException e) {
      throw new AdminException(e.getMessage());
    }

    try{
      Iterator i = config.getSectionChildSectionNames(Network.NETWORKS_PREFIX).iterator();
      while(i.hasNext()) {
        String s = (String)i.next();
        int n = Integer.parseInt(s.substring(s.lastIndexOf(".")+1));
        networks.put(new Integer(n), Network.loadFromConfig(n, config));
      }
    }catch(Exception e) {
      logger.error(e, e);
      throw new AdminException(e.getMessage());
    }

  }

  private void initIdFile(String configDir) throws IOException{
    File f = new File(configDir, "networkid.bin");
    if (!f.exists()) {
      File parentDir = new File(configDir).getParentFile();
      if (parentDir != null) {
        File f1 = new File(parentDir, "networkid.bin");
        if (f1.exists())
          f = f1;
      }
    }
    idFile = new RandomAccessFile(f, "rw");

    long len = idFile.length();
    if (len % 4 > 0) {
      logger.warn("File networkid.bin is broken and will be repaired.");
      Utils.truncateFile(idFile, 4 * len / 4 - 1);
      idFile.close();
      idFile = new RandomAccessFile(new File(configDir, "taskid.bin"), "rw");
      logger.warn("File networkid.bin was successfully repaired.");
    }

    // Load id
    if (len == 0)
      id = 0;
    else {
      idFile.seek(idFile.length() - 4);
      id = idFile.readInt();
    }
  }

  private synchronized Integer getId() throws AdminException {
    try {
      id++;
      idFile.seek(idFile.length());
      idFile.writeInt(id);
      return new Integer(id);
    } catch (IOException e) {
      logger.error(e, e);
      throw new AdminException(e.getMessage());
    }
  }

  synchronized Collection getNetworks() {
    return networks.values();
  }

  synchronized Network getNetwork(int id) {
    return (Network)networks.get(new Integer(id));
  }

  synchronized void saveNetwork(Network n) throws AdminException, Config.WrongParamTypeException, IOException {
    if(n == null) {
      throw new AdminException("Network is null");
    }
    if(n.getName() == null || n.getName().trim().length() == 0) {
      throw new AdminException("Network name is empty");
    }
    if(n.getMcc()<100 || n.getMcc()>999) {
      throw new AdminException("Incorrect mcc");
    }
    if(n.getMnc()<10 || n.getMnc()>99) {
      throw new AdminException("Incorrect mnc");
    }
    if(n.getCountryId() == null || n.getCountryId().trim().length() == 0) {
      throw new AdminException("Please select country");
    }
    Iterator i = networks.values().iterator();
    while(i.hasNext()) {
      Network network = (Network)i.next();
      if(network.getName().equals(n.getName()) && (n.getId() == null || n.getId().intValue() != network.getId().intValue())) {
        throw new AdminException("Network with such name already exists: "+n.getName());
      }
      if(network.getMcc() == n.getMcc() && network.getMnc() == n.getMnc() && (n.getId() == null || n.getId().intValue() != network.getId().intValue())) {
        throw new AdminException("Pair of mcc and mnc is already exist. Network: "+network.getName());
      }
    }
    if(n.getId() == null) {
      n.setId(getId());
    }
    networks.put(n.getId(), n);
    n.save(config);
  }

  synchronized void removeNetworks(Collection ids) throws Config.WrongParamTypeException, IOException {
    if(ids == null) {
      return;
    }
    Iterator i = ids.iterator();
    while(i.hasNext()) {
      Integer id = (Integer)i.next();
      String sectionName = new StringBuffer().append(Network.NETWORKS_PREFIX).append('.').append(id).toString();
      if(networks.containsKey(id) && config.containsSection(sectionName)) {
        config.removeSection(sectionName);
        networks.remove(id);
      }

    }
    config.save();
  }
}
