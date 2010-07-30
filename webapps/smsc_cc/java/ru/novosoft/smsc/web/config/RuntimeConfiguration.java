package ru.novosoft.smsc.web.config;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.alias.AliasSet;
import ru.novosoft.smsc.util.Address;

/**
 * @author Artem Snopkov
 */
public class RuntimeConfiguration {

  private final AdminContext adminContext;

  public RuntimeConfiguration(AdminContext adminContext) {
    this.adminContext = adminContext;
  }

  public void addAlias(Alias alias) throws AdminException {
    adminContext.getAliasManager().addAlias(alias);
  }

  public void deleteAlias(Address alias) throws AdminException {
    adminContext.getAliasManager().deleteAlias(alias);
  }

  public AliasSet getAliases() throws AdminException {
    return adminContext.getAliasManager().getAliases();
  }
  

}
