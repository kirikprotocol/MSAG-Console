package ru.sibinco.smsx.engine.service.group.datasource.impl.principal;

import ru.sibinco.smsx.engine.service.group.datasource.Principal;
import ru.sibinco.smsx.engine.service.group.datasource.PrincipalDataSource;
import ru.sibinco.smsx.utils.DataSourceException;

import java.io.*;
import java.util.HashMap;
import java.util.Map;
import java.util.Collection;
import java.util.ArrayList;

/**
 * User: artem
 * Date: 18.11.2008
 */
public class FilePrincipalDataSource implements PrincipalDataSource {

  private final PrincipalsFile file;
  private final Map<String, PrincipalWrapper> principals;

  public FilePrincipalDataSource(File file) throws DataSourceException {
    try {
      this.file = new PrincipalsFile(file);
      this.principals = new HashMap<String, PrincipalWrapper>(this.file.getPrincipalsNumber() + 100);
      this.file.list(new PrincipalsFile.Visitor() {
        public void principal(Principal p, long offset) {
          principals.put(p.getAddress(), new PrincipalWrapper(p, offset));
        }
      });
    } catch (IOException e) {
      throw new DataSourceException(e);
    }
  }

  public void addPrincipal(Principal principal) throws DataSourceException {
    try {
      long offset = file.addPrincipal(principal);
      principals.put(principal.getAddress(), new PrincipalWrapper(principal, offset));
    } catch(IOException e){
      throw new DataSourceException(e);
    }
  }

  public boolean removePrincipal(String address) throws DataSourceException {
    PrincipalWrapper w = principals.get(address);
    if (w == null)
      return false;

    try {
      file.removePrincipal(w.fileOffset);
      principals.remove(address);
      return true;
    } catch (IOException e) {
      throw new DataSourceException(e);
    }
  }

  public Principal getPrincipal(String address) throws DataSourceException {
    PrincipalWrapper w = principals.get(address);
    return (w==null) ? null : w.principal;
  }

  public Collection<Principal> getPrincipals() throws DataSourceException {
    Collection<Principal> result = new ArrayList<Principal>(principals.size());
    for (PrincipalWrapper w : principals.values())
      result.add(w.principal);
    return result;
  }

  public boolean containsPrincipal(String address) throws DataSourceException {
    return principals.containsKey(address);
  }

  public void close() {
    file.close();
  }
}
