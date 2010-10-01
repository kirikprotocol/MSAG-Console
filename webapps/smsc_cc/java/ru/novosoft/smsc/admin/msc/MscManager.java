package ru.novosoft.smsc.admin.msc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.util.Address;

import java.util.Collection;

/**
 * @author Artem Snopkov
 */
public interface MscManager extends SmscConfiguration {

  Collection<Address> mscs() throws AdminException;

  void addMsc(Address msc) throws AdminException;

  void removeMsc(Address msc) throws AdminException;
}
