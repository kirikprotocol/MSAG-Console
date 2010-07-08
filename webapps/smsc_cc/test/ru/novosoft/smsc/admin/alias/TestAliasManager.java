package ru.novosoft.smsc.admin.alias;

import ru.novosoft.smsc.admin.AdminException;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;

/**
 * Тестовая реализация AliasManager.
 * @author Artem Snopkov
 */
public class TestAliasManager extends AliasManager {

  private final Collection<Alias> aliases;

  public TestAliasManager() {
    aliases = new ArrayList<Alias>(100);
    Collections.addAll(aliases,
        new Alias("12345", "54321", false),
        new Alias("10000", ".5.0.MTC", false),
        new Alias("1232", ".5.0.MMC", true));
  }

  public void addAlias(Alias alias) throws AdminException {
    if (aliases.contains(alias))
      throw new AliasManagerException("alias_already_exists", alias.toString());
    aliases.add(alias);
  }

  public void deleteAlias(Alias alias) throws AdminException {
    if (!aliases.remove(alias))
      throw new AliasManagerException("alias_does_not_exists", alias.toString());
  }

  public AliasSet getAliases() throws AdminException {
    final Iterator<Alias> iterator = aliases.iterator();
    return new AliasSet() {
      private Alias cur;

      public boolean next() {
        if (iterator.hasNext()) {
          cur=iterator.next();
          return true;
        }
        return false;
      }

      public Alias get() {
        return cur;
      }

      public void close() {
      }
    };
  }
}
