package ru.novosoft.smsc.admin.smsc_service;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.route.SMEList;

import java.util.List;
import java.util.Collection;

/**
 * Created by igork
 * Date: Jan 20, 2003
 * Time: 6:40:28 PM
 */
public interface SmeManager
{
	SME add(String id, int priority, byte type, int typeOfNumber, int numberingPlan, int interfaceVersion, String systemType,
			  String password, String addrRange, int smeN, boolean wantAlias, boolean forceDC, int timeout)
			throws AdminException;

	SME add(SME newSme) throws AdminException;

	SME get(String id) throws AdminException;

	SME remove(String id) throws AdminException;

	boolean contains(String id);

	void removeAllIfSme(Collection serviceIds) throws AdminException;

	List getSmeNames();
	SMEList getSmes();

	void save() throws AdminException;
}
