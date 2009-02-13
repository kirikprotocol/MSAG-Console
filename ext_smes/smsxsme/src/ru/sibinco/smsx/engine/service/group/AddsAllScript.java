package ru.sibinco.smsx.engine.service.group;

import ru.sibinco.smsx.engine.service.group.datasource.impl.distrlist.FileDistrListDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.DistrListDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.DistrList;
import ru.sibinco.smsx.utils.DataSourceException;

import java.io.File;

/**
 * User: artem
 * Date: 15.12.2008
 *
 * Script transfers all members to submitters
 */
public class AddsAllScript {

  private static String prepareMsisdn(String msisdn) {
    if (msisdn.charAt(0) == '8' && msisdn.length() == 11)
      msisdn = "+7" + msisdn.substring(1);
    return msisdn;
  }

  public static void main(String args[]) throws DataSourceException {
    if (args.length == 0) {
      System.out.println("Usage: <store dir>");
      return;
    }

    String storeDir = args[0];

    System.out.println("Loading distr lists...");
    DistrListDataSource listsDS = new FileDistrListDataSource(new File(storeDir, "members.bin"), new File(storeDir, "submitters.bin"), new File(storeDir, "lists.bin"));
    DistrListDataSource listsDS1 = new FileDistrListDataSource(new File(storeDir, "members_new.bin"), new File(storeDir, "submitters_new.bin"), new File(storeDir, "lists_new.bin"));
    System.out.println("Distr lists have been loaded.");

    for (DistrList dl : listsDS.getDistrLists(null)) {
      DistrList dlnew = listsDS1.createDistrList(dl.getName(), dl.getOwner(), dl.getMaxElements());
      dlnew.addSubmitter(dl.getOwner());

      for (String member : dl.members()) {
        System.out.println("Add " + member + " to " + dl.getName() + " submitters and members");
        String m = prepareMsisdn(member);
        if (!dlnew.containsSubmitter(m))
          dlnew.addSubmitter(m);
        if (!dlnew.containsMember(m))
          dlnew.addMember(m);
      }

      for (String submitter : dl.submitters()) {
        String s = prepareMsisdn(submitter);
        if (!dlnew.containsSubmitter(s)) {
          System.out.println("Add " + submitter + " to " + dl.getName() + " submitters");
          dlnew.addSubmitter(s);
        }
      }
    }

    listsDS.close();
    listsDS1.close();
  }
}
