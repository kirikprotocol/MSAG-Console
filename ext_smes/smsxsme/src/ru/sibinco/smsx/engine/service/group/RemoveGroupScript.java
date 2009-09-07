package ru.sibinco.smsx.engine.service.group;

import ru.sibinco.smsx.engine.service.group.datasource.DistrListDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.impl.distrlist.FileDistrListDataSource;
import ru.sibinco.smsx.utils.DataSourceException;

import java.io.File;

/**
 * User: Артем
 * Date: 07.09.2009
 */
public class RemoveGroupScript {

  public static void main(String[] args) throws DataSourceException {
    String storeDir = args[0];
    String owner = args[1];

    DistrListDataSource listsDS = new FileDistrListDataSource(new File(storeDir, "members.bin"), new File(storeDir, "submitters.bin"), new File(storeDir, "lists.bin"));

    listsDS.removeDistrLists(owner);
  }
}
