package ru.novosoft.smsc.admin.closed_groups;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Address;

import java.util.Collection;

/**
 * @author Aleksandr Khalitov
 */
public interface ClosedGroup {

  public int getId() throws AdminException;

  public String getName() throws AdminException;

  public String getDescription() throws AdminException;

  public void setDescription(String description) throws AdminException;

  public Collection<Address> getMasks() throws AdminException;

  public void removeMask(Address mask) throws AdminException;

  public void addMask(Address mask) throws AdminException;  

}
