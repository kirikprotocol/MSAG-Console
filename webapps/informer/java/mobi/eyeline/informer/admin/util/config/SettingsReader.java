package mobi.eyeline.informer.admin.util.config;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 13.10.2010
 * Time: 14:42:06
 */
public interface SettingsReader<C,T> {
    public T executeRead(C settings);
}
