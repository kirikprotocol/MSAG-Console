package ru.novosoft.smsc.admin.stat;

import ru.novosoft.smsc.admin.util.DBExportSettings;

/**
 * author: Aleksandr Khalitov
 */
public class TestSmscStatProvider extends SmscStatProvider{

  public TestSmscStatProvider(SmscStatContext context, DBExportSettings defExportSettings) {
    super(context, defExportSettings);
  }
}
