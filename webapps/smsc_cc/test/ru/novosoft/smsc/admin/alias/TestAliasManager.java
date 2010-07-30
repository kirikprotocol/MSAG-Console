package ru.novosoft.smsc.admin.alias;

import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.IOUtils;

import java.io.*;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

/**
 * Тестовая реализация AliasManager.
 *
 * @author Artem Snopkov
 */
public class TestAliasManager extends AliasManager {

  private static final int MSG_SIZE = 48; //1+(1+1+21)+(1+1+21)+1

  public TestAliasManager(File aliasesFile, ClusterController clusterController, FileSystem fileSystem) {
    super(aliasesFile, clusterController, fileSystem);
  }

  private static Collection<Alias> readAliases(File aliasesFile) {
    Collection<Alias> aliases = new ArrayList<Alias>();
    InputStream is = null;
    try {
      is = new BufferedInputStream(new FileInputStream(aliasesFile));

      IOUtils.readString(is, 8); // file name
      IOUtils.readUInt32(is); // version (int)

      while (true) {
        while (IOUtils.readUInt8(is) == 0)
          IOUtils.skip(is, MSG_SIZE - 1);

        byte addrTon = (byte) IOUtils.readUInt8(is);
        byte addrNpi = (byte) IOUtils.readUInt8(is);
        String addrValue = IOUtils.readString(is, 21);
        byte aliasTon = (byte) IOUtils.readUInt8(is);
        byte aliasNpi = (byte) IOUtils.readUInt8(is);
        String aliasValue = IOUtils.readString(is, 21);
        byte isHide = (byte) IOUtils.readUInt8(is);

        aliases.add(new Alias(new Address(addrTon, addrNpi, addrValue), new Address(aliasTon, aliasNpi, aliasValue), isHide != 0));
      }

    } catch (IOException e) {
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (IOException e) {
        }
    }

    return aliases;
  }

  private static void saveAliases(File aliasesFile, Collection<Alias> aliases) {
    OutputStream os = null;
    try {
      os = new BufferedOutputStream(new FileOutputStream(aliasesFile));

      IOUtils.writeString(os, "ALIASMAN", 8);
      IOUtils.writeUInt32(os, 12);

      for (Alias a: aliases) {
        Address address = a.getAddress();
        Address alias = a.getAlias();

        IOUtils.writeUInt8(os, 1);

        IOUtils.writeUInt8(os, address.getTone());
        IOUtils.writeUInt8(os, address.getNpi());
        IOUtils.writeString(os, address.getAddress(), 21);

        IOUtils.writeUInt8(os, alias.getTone());
        IOUtils.writeUInt8(os, alias.getNpi());
        IOUtils.writeString(os, alias.getAddress(), 21);

        IOUtils.writeUInt8(os, a.isHide() ? 0 : 1);
      }

    } catch (IOException e) {
    } finally {
      if (os != null)
        try {
          os.close();
        } catch (IOException e) {
        }
    }
  }

  public static void helpAddAlias(File aliasesFile, Address address, Address alias, boolean hide) {
    Alias a = new Alias(address, alias, hide);
    Collection<Alias> aliases = readAliases(aliasesFile);
    aliases.remove(a);
    aliases.add(a);
    saveAliases(aliasesFile, aliases);
  }

  public static void helpRemoveAlias(File aliasesFile, Address alias) {
    Collection<Alias> aliases = readAliases(aliasesFile);
    for (Iterator<Alias> iter = aliases.iterator(); iter.hasNext();) {
      if (iter.next().getAlias().equals(alias)) {
        iter.remove();
        break;
      }
    }


    saveAliases(aliasesFile, aliases);
  }
}
