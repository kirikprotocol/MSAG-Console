package ru.novosoft.smsc.admin.resource_group;

import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.AdminException;

import java.util.*;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 09.09.2005
 * Time: 15:40:03
 * To change this template use File | Settings | File Templates.
 */

public abstract class ResourceGroupImpl implements ResourceGroup
{
	protected String name;
	
	protected String[] nodes;

	protected Category logger = Category.getInstance(this.getClass().getName());

	protected long swigCPtr;
	protected boolean swigCMemOwn;

	protected ResourceGroupImpl(long cPtr, boolean cMemoryOwn) {
		swigCMemOwn = cMemoryOwn;
		swigCPtr = cPtr;
	}

	public String getName()
	{
		return name;
	}
}
