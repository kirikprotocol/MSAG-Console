package ru.sibinco.smsx.engine.service.group.datasource;

import ru.sibinco.smsx.engine.service.group.datasource.impl.distrlist.DBDistrListDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.impl.distrlist.FileDistrListDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.impl.principal.DBPrincipalDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.impl.principal.FilePrincipalDataSource;
import ru.sibinco.smsx.utils.DataSourceException;
import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;

import java.io.File;
import java.util.Collection;

import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.ConfigException;

/**
 * User: artem
 * Date: 02.12.2009
 */
public class Convertor {

  public static void main(String args[]) throws DataSourceException, ConfigException {

    final XmlConfig config = new XmlConfig();
    config.load(new File("conf", "config.xml"));

    String storeDir = config.getSection("group").getString("store.dir");

    // Init DB connection pool
    System.out.println("Initiate DB Connection pool...");
    ConnectionPoolFactory.init(config);

    System.out.println("Initiate distr list storages...");
    DistrListDataSource oldListsDS = new FileDistrListDataSource(new File(storeDir, "members.bin"), new File(storeDir, "submitters.bin"), new File(storeDir, "lists.bin"));
    DistrListDataSource newListsDS = new DBDistrListDataSource();

    System.out.println("Initiate principal storages...");
    PrincipalDataSource oldPrincipalDS = new FilePrincipalDataSource(new File(storeDir, "principals.bin"));
    PrincipalDataSource newPrincipalDS = new DBPrincipalDataSource();

    System.out.println("Initialization completed");

    // Copy lists
    int listsCount = 0;
    Collection<DistrList> lists = oldListsDS.getDistrLists(null);
    for (DistrList oldList : lists) {
      System.out.print("Converting list" + oldList.getOwner() + "/" + oldList.getName() + " ...");
      DistrList newList = newListsDS.createDistrList(oldList.getName(), oldList.getOwner(), oldList.getMaxElements());
      newList.copyFrom(oldList);
      System.out.println(" OK");
      listsCount++;
    }

    // Copy principals
    int principalsCount=0;
    Collection<Principal> principals = oldPrincipalDS.getPrincipals();
    for (Principal oldPrincipal : principals) {
      System.out.print("Converting principal: " + oldPrincipal.getAddress() + " ...");
      newPrincipalDS.addPrincipal(oldPrincipal);
      System.out.println(" OK");
      principalsCount++;
    }

    System.out.println("Conversion completed: lists=" + listsCount + ", principals=" + principalsCount);

    oldListsDS.close();
    newListsDS.close();

    oldPrincipalDS.close();
    newPrincipalDS.close();        
  }
}
