package mobi.eyeline.informer.admin.infosme.protogen;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.infosme.Category;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.infosme.InfosmeException;
import mobi.eyeline.informer.admin.infosme.protogen.protocol.*;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class InfosmeImpl implements Infosme {

  private InfosmeClient client;

  public InfosmeImpl(String host, int port) {
    this.client = new InfosmeClient(host, port);
  }

  protected void checkResponse(int status) throws InfosmeException {
    if (status != 0) {
      throw new InfosmeException("interaction_error", status + "");
    }
  }

  public void addSmsc(String smscId) throws AdminException{
    if(smscId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciSmsc);
    configOp.setOp(ConfigOpId.coAdd);
    configOp.setObjId(smscId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void removeSmsc(String smscId) throws AdminException {
    if(smscId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciSmsc);
    configOp.setOp(ConfigOpId.coRemove);
    configOp.setObjId(smscId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void updateSmsc(String smscId) throws AdminException {
    if(smscId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciSmsc);
    configOp.setOp(ConfigOpId.coUpdate);
    configOp.setObjId(smscId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void addRegion(String regionId) throws AdminException {
    if(regionId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciRegion);
    configOp.setOp(ConfigOpId.coAdd);
    configOp.setObjId(regionId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void updateRegion(String regionId) throws AdminException {
    if(regionId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciRegion);
    configOp.setOp(ConfigOpId.coUpdate);
    configOp.setObjId(regionId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void removeRegion(String regionId) throws AdminException {
    if(regionId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciRegion);
    configOp.setOp(ConfigOpId.coRemove);
    configOp.setObjId(regionId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void addUser(String userId) throws AdminException {
    if(userId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciUser);
    configOp.setOp(ConfigOpId.coAdd);
    configOp.setObjId(userId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void updateUser(String userId) throws AdminException {
    if(userId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciUser);
    configOp.setOp(ConfigOpId.coUpdate);
    configOp.setObjId(userId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void removeUser(String userId) throws AdminException {
    if(userId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciUser);
    configOp.setOp(ConfigOpId.coRemove);
    configOp.setObjId(userId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void addRetryPolicy(String policyId) throws AdminException {
    if(policyId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciRetry);
    configOp.setOp(ConfigOpId.coAdd);
    configOp.setObjId(policyId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void updateRetryPolicy(String policyId) throws AdminException {
    if(policyId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciRetry);
    configOp.setOp(ConfigOpId.coUpdate);
    configOp.setObjId(policyId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void removeRetryPolicy(String policyId) throws AdminException {
    if(policyId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciRetry);
    configOp.setOp(ConfigOpId.coRemove);
    configOp.setObjId(policyId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void setDefaultSmsc(String smscId) throws AdminException {
    if(smscId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    SetDefaultSmsc s = new SetDefaultSmsc();
    s.setId(smscId);
    checkResponse(client.send(s).getStatus());
  }

  public void setCategories(List<Category> categories) throws AdminException {
    if(categories == null) {
      throw new IllegalArgumentException("Categories is null");
    }
    CategoryInfo[] cs = new CategoryInfo[categories.size()];
    int i = 0;
    for(Category category : categories) {
      CategoryInfo c = new CategoryInfo();
      c.setLevel(category.getLevel());
      c.setName(category.getName());
      cs[i] = c;
      i++;
    }

    LoggerSetCategories setCategories = new LoggerSetCategories();
    setCategories.setCategories(cs);

    checkResponse(client.send(setCategories).getStatus());
  }

  public List<Category> getCategories() throws AdminException {
    LoggerGetCategories getCategories =  new LoggerGetCategories();
    LoggerGetCategoriesResp resp = client.send(getCategories);
    checkResponse(resp.getStatus());
    if(resp.hasCategories()) {
      CategoryInfo[] cs = resp.getCategories();
      List<Category> result = new ArrayList<Category>(cs.length);
      for(CategoryInfo ci : cs) {
        Category category = new Category();
        category.setLevel(ci.getLevel());
        category.setName(ci.getName());
        result.add(category);
      }
      return result;
    }else {
      return Collections.emptyList();
    }
  }

  public boolean isOnline() throws AdminException {
    return client.isConnected();
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  public void shutdown() {
    if(client != null) {
      try{         
        client.shutdown();
      }catch (Exception e){}
    }
  }



}
