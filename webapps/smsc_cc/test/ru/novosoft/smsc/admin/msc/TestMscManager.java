package ru.novosoft.smsc.admin.msc;

import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Address;

import java.io.*;
import java.util.ArrayList;
import java.util.Collection;

/**
 * @author Artem Snopkov
 */
public class TestMscManager extends MscManager {

  public TestMscManager(File aliasesFile, ClusterController cc, FileSystem fs) {
    super(aliasesFile, cc, fs);
  }

  private static Collection<Address> readMscs(File file) {
    Collection<Address> res = new ArrayList<Address>();
    BufferedReader r = null;
    try {
      r = new BufferedReader(new FileReader(file));
      String line;
      while ((line = r.readLine()) != null) {
        if (line.trim().length() > 0)
          res.add(new Address(line));
      }

    } catch (IOException e) {
    } finally {
      if (r != null)
        try {
          r.close();
        } catch (IOException e) {
        }
    }
    return res;
  }

  private static void saveMscs(File toFile, Collection<Address> mscs) {
    BufferedWriter w = null;
    try {
      w = new BufferedWriter(new FileWriter(toFile));

      for (Address a : mscs) {
        w.write(a.getNormalizedAddress());
        w.newLine();
      }
    } catch (IOException e) {
    } finally {
      if (w != null)
        try {
          w.close();
        } catch (IOException e) {
        }
    }
  }

  public static void helpAddMsc(File file, Address msc) {
    Collection<Address> mscs = readMscs(file);
    mscs.remove(msc);
    mscs.add(msc);
    saveMscs(file, mscs);
  }

  public static void helpRemoveMsc(File file, Address msc) {
    Collection<Address> mscs = readMscs(file);
    mscs.remove(msc);
    saveMscs(file, mscs);
  }
}
