package mobi.eyeline.informer.admin.util.config;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.infosme.Infosme;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 13.10.2010
 * Time: 14:33:30
 */
public interface SettingsWriter<C> {

    void changeSettings(C settings) throws AdminException ;

    void infosmeCommand(Infosme infosme) throws AdminException ;

  }