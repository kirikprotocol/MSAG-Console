package ru.novosoft.smsc.admin;

import ru.novosoft.smsc.admin.resource.ResourceManager;
import ru.novosoft.smsc.admin.sme.SmeManager;
import ru.novosoft.smsc.admin.smsc.SmscManager;

import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class AdminContextLocator {

  private final static List<AdminContext> contexts = new ArrayList<AdminContext>();

  static void registerContext(AdminContext ctx) {
    contexts.add(ctx);
  }

  static void unregisterContext(AdminContext ctx) {
    contexts.remove(ctx);
  }

  public static AdminContext getContext(SmscManager m) {
    for (AdminContext c : contexts) {
      if (c.getSmscManager() == m)
        return c;
    }
    return null;
  }

  public static AdminContext getContext(ResourceManager m) {
    for (AdminContext c : contexts) {
      if (c.getResourceManager() == m)
        return c;
    }
    return null;
  }

  public static AdminContext getContext(SmeManager m) {
    for (AdminContext c : contexts) {
      if (c.getSmeManager() == m)
        return c;
    }
    return null;
  }

}
