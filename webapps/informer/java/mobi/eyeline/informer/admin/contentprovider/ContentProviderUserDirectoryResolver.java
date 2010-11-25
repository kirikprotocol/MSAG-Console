package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.users.User;

import java.io.File;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 25.11.2010
 * Time: 13:03:07
 */
public interface ContentProviderUserDirectoryResolver {
  File getUserDirectory(User user) throws AdminException;
}
