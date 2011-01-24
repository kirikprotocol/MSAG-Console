package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;

import java.io.File;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 30.11.2010
 * Time: 18:32:46
 */
public class TestContentProviderDaemon extends ContentProviderDaemon{


  public TestContentProviderDaemon(ContentProviderContext context, File informerBase, File appWorkDir) throws AdminException {
    super(context, informerBase, appWorkDir);
  }
}
