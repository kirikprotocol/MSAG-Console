package ru.novosoft.smsc.proxysme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.*;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.Functions;

import javax.servlet.http.HttpServletRequest;
import java.security.Principal;
import java.util.HashMap;
import java.util.List;

/**
 * Created by igork
 * Date: Aug 5, 2003
 * Time: 3:52:08 PM
 */
public class Index extends PageBean {
	protected static final int PRIVATE_RESULT = PageBean.PRIVATE_RESULT + 1;

	private Config config = null;

	private String left_host = "";
	private int left_port = 0;
	private String left_sid = "";
	private String left_password = "";
	private String left_origAddr = "";
	private String left_systemType = "";
	private int left_timeOut = 0;
	private int left_defaultDCS = 0;

	private String right_host = "";
	private int right_port = 0;
	private String right_sid = "";
	private String right_password = "";
	private String right_origAddr = "";
	private String right_systemType = "";
	private int right_timeOut = 0;
	private int right_defaultDCS = 0;

	private int queueLength = 0;
	private String adminHost = "";
	private int adminPort = 0;

	private boolean initialized = false;
	private String mbApply = null;
	private String mbClear = null;
	private String mbStart = null;
	private String mbStop = null;

  private String proxySmeId = "";

	public int process(HttpServletRequest request)
	{
		int result = super.process(request);
		if (result != RESULT_OK)
			return result;

    try {
      proxySmeId = Functions.getServiceId(request.getServletPath());
    } catch (AdminException e) {
      logger.error("Internal error", e);
      return error("Internal error", e);
    }

    try {
			config = ProxySmeContext.getInstance(appContext, proxySmeId).getConfig();
		} catch (Throwable e) {
			logger.debug("Couldn't get config", e);
			return error("Couldn't get config", e);
		}

		if (!initialized) {
			result = initializeFromConfig();
			if (result != RESULT_OK)
				return result;
		} else
			setConfigFromParams();

		if (mbClear != null)
			return clear(appContext);
		if (mbApply != null)
			return apply();
		if (mbStart != null)
			return start();
		if (mbStop != null)
			return stop();


		return result;
	}

	private int start()
	{
		try {
			appContext.getHostsManager().startService(proxySmeId);
			return RESULT_DONE;
		} catch (AdminException e) {
			logger.debug("Couldn't start service", e);
			return error("Couldn't start service", e);
		}
	}

	private int stop()
	{
		try {
			appContext.getHostsManager().shutdownService(proxySmeId);
			return RESULT_DONE;
		} catch (AdminException e) {
			logger.debug("Couldn't stop service", e);
			return error("Couldn't stop service", e);
		}
	}

	private void setConfigFromParams()
	{
		config.setString("left.host", left_host);
		config.setInt("left.port", left_port);
		config.setString("left.sid", left_sid);
		config.setString("left.password", left_password);
		config.setString("left.origAddr", left_origAddr);
		config.setString("left.systemType", left_systemType);
		config.setInt("left.timeOut", left_timeOut);
		config.setInt("left.defaultDCS", left_defaultDCS);

		config.setString("right.host", right_host);
		config.setInt("right.port", right_port);
		config.setString("right.sid", right_sid);
		config.setString("right.password", right_password);
		config.setString("right.origAddr", right_origAddr);
		config.setString("right.systemType", right_systemType);
		config.setInt("right.timeOut", right_timeOut);
		config.setInt("right.defaultDCS", right_defaultDCS);

		config.setInt("queueLength", queueLength);
		config.setString("adminHost", adminHost);
		config.setInt("adminPort", adminPort);
	}

	private int clear(SMSCAppContext appContext)
	{
		try {
			config = ProxySmeContext.getInstance(appContext, proxySmeId).loadConfig();
			int result = initializeFromConfig();
			return result == RESULT_OK ? RESULT_DONE : result;
		} catch (Throwable e) {
			logger.debug("Couldn't reload config", e);
			return error("Couldn't reload config", e);
		}
	}

	private int apply()
	{
		logger.debug("APPLY");
		try {
			config.save();
		} catch (Throwable e) {
			logger.debug("Couldn't save config", e);
			return error("Couldn't save config", e);
		}

		if (isSmeRunning()) {
			try {
				Service service = appContext.getHostsManager().getService(proxySmeId);
				service.refreshComponents();
				Component component = (Component) service.getInfo().getComponents().get("ProxySme");
				service.call(component, (Method) component.getMethods().get("applay"), Type.Types[Type.StringType], new HashMap());
			} catch (Throwable e) {
				logger.debug("Couldn't call apply", e);
				return error("Couldn't call apply", e);
			}
		}

		return RESULT_DONE;
	}

	private int initializeFromConfig()
	{
		try {
			left_host = config.getString("left.host");
			left_port = config.getInt("left.port");
			left_sid = config.getString("left.sid");
			left_password = config.getString("left.password");
			left_origAddr = config.getString("left.origAddr");
			left_systemType = config.getString("left.systemType");
			left_timeOut = config.getInt("left.timeOut");
			left_defaultDCS = config.getInt("left.defaultDCS");

			right_host = config.getString("right.host");
			right_port = config.getInt("right.port");
			right_sid = config.getString("right.sid");
			right_password = config.getString("right.password");
			right_origAddr = config.getString("right.origAddr");
			right_systemType = config.getString("right.systemType");
			right_timeOut = config.getInt("right.timeOut");
			right_defaultDCS = config.getInt("right.defaultDCS");

			queueLength = config.getInt("queueLength");
			adminHost = config.getString("adminHost");
			adminPort = config.getInt("adminPort");

			return RESULT_OK;
		} catch (Throwable e) {
			logger.debug("Couldn't get paramteters from config", e);
			left_host = "";
			left_port = 0;
			left_sid = "";
			left_password = "";
			left_origAddr = "";
			left_systemType = "";
			left_timeOut = 0;
			left_defaultDCS = 0;

			right_host = "";
			right_port = 0;
			right_sid = "";
			right_password = "";
			right_origAddr = "";
			right_systemType = "";
			right_timeOut = 0;
			right_defaultDCS = 0;

			queueLength = 0;
			adminHost = "";
			adminPort = 0;
			return error("Couldn't get parameters from config", e);
		}
	}

	public String getSmeId()
	{
		return proxySmeId;
	}

	public boolean isSmeRunning()
	{
		try {
			return appContext.getHostsManager().getServiceInfo(proxySmeId).getStatus() == ServiceInfo.STATUS_RUNNING;
		} catch (AdminException e) {
			logger.debug("Couldn't get proxysme status", e);
			return false;
		}
	}

	public String getLeft_host()
	{
		return left_host;
	}

	public void setLeft_host(String left_host)
	{
		this.left_host = left_host;
	}

	public String getLeft_port()
	{
		return String.valueOf(left_port);
	}

	public void setLeft_port(String left_port)
	{
		try {
			this.left_port = Integer.decode(left_port).intValue();
		} catch (NumberFormatException e) {
			this.left_port = 0;
		}
	}

	public int getLeft_portInt()
	{
		return left_port;
	}

	public void setLeft_portInt(int left_port)
	{
		this.left_port = left_port;
	}

	public String getLeft_sid()
	{
		return left_sid;
	}

	public void setLeft_sid(String left_sid)
	{
		this.left_sid = left_sid;
	}

	public String getLeft_password()
	{
		return left_password;
	}

	public void setLeft_password(String left_password)
	{
		this.left_password = left_password;
	}

	public String getLeft_origAddr()
	{
		return left_origAddr;
	}

	public void setLeft_origAddr(String left_origAddr)
	{
		this.left_origAddr = left_origAddr;
	}

	public String getLeft_systemType()
	{
		return left_systemType;
	}

	public void setLeft_systemType(String left_systemType)
	{
		this.left_systemType = left_systemType;
	}

	public String getLeft_timeOut()
	{
		return String.valueOf(left_timeOut);
	}

	public void setLeft_timeOut(String left_timeOut)
	{
		try {
			this.left_timeOut = Integer.decode(left_timeOut).intValue();
		} catch (NumberFormatException e) {
			this.left_timeOut = 0;
		}
	}

	public int getLeft_timeOutInt()
	{
		return left_timeOut;
	}

	public void setLeft_timeOutInt(int left_timeOut)
	{
		this.left_timeOut = left_timeOut;
	}

	public int getLeft_defaultDCS()
	{
		return left_defaultDCS;
	}

	public void setLeft_defaultDCS(int left_defaultDCS)
	{
		this.left_defaultDCS = left_defaultDCS;
	}

	public String getRight_host()
	{
		return right_host;
	}

	public void setRight_host(String right_host)
	{
		this.right_host = right_host;
	}

	public String getRight_port()
	{
		return String.valueOf(right_port);
	}

	public void setRight_port(String right_port)
	{
		try {
			this.right_port = Integer.decode(right_port).intValue();
		} catch (NumberFormatException e) {
			this.right_port = 0;
		}
	}

	public int getRight_portInt()
	{
		return right_port;
	}

	public void setRight_portInt(int right_port)
	{
		this.right_port = right_port;
	}

	public String getRight_sid()
	{
		return right_sid;
	}

	public void setRight_sid(String right_sid)
	{
		this.right_sid = right_sid;
	}

	public String getRight_password()
	{
		return right_password;
	}

	public void setRight_password(String right_password)
	{
		this.right_password = right_password;
	}

	public String getRight_origAddr()
	{
		return right_origAddr;
	}

	public void setRight_origAddr(String right_origAddr)
	{
		this.right_origAddr = right_origAddr;
	}

	public String getRight_systemType()
	{
		return right_systemType;
	}

	public void setRight_systemType(String right_systemType)
	{
		this.right_systemType = right_systemType;
	}

	public String getRight_timeOut()
	{
		return String.valueOf(right_timeOut);
	}

	public void setRight_timeOut(String right_timeOut)
	{
		try {
			this.right_timeOut = Integer.decode(right_timeOut).intValue();
		} catch (NumberFormatException e) {
			this.right_timeOut = 0;
		}
	}

	public int getRight_timeOutInt()
	{
		return right_timeOut;
	}

	public void setRight_timeOutInt(int right_timeOut)
	{
		this.right_timeOut = right_timeOut;
	}

	public int getRight_defaultDCS()
	{
		return right_defaultDCS;
	}

	public void setRight_defaultDCS(int right_defaultDCS)
	{
		this.right_defaultDCS = right_defaultDCS;
	}

	public String getQueueLength()
	{
		return String.valueOf(queueLength);
	}

	public void setQueueLength(String queueLength)
	{
		try {
			this.queueLength = Integer.decode(queueLength).intValue();
		} catch (NumberFormatException e) {
			this.queueLength = 0;
		}
	}

	public int getQueueLengthInt()
	{
		return queueLength;
	}

	public void setQueueLengthInt(int queueLength)
	{
		this.queueLength = queueLength;
	}

	public String getAdminHost()
	{
		return adminHost;
	}

	public void setAdminHost(String adminHost)
	{
		this.adminHost = adminHost;
	}

	public String getAdminPort()
	{
		return String.valueOf(adminPort);
	}

	public void setAdminPort(String adminPort)
	{
		try {
			this.adminPort = Integer.decode(adminPort).intValue();
		} catch (NumberFormatException e) {
			this.adminPort = 0;
		}
	}

	public int getAdminPortInt()
	{
		return adminPort;
	}

	public void setAdminPortInt(int adminPort)
	{
		this.adminPort = adminPort;
	}

	public boolean isInitialized()
	{
		return initialized;
	}

	public void setInitialized(boolean initialized)
	{
		this.initialized = initialized;
	}

	public String getMbApply()
	{
		return mbApply;
	}

	public void setMbApply(String mbApply)
	{
		this.mbApply = mbApply;
	}

	public String getMbClear()
	{
		return mbClear;
	}

	public void setMbClear(String mbClear)
	{
		this.mbClear = mbClear;
	}

	public String getMbStart()
	{
		return mbStart;
	}

	public void setMbStart(String mbStart)
	{
		this.mbStart = mbStart;
	}

	public String getMbStop()
	{
		return mbStop;
	}

	public void setMbStop(String mbStop)
	{
		this.mbStop = mbStop;
	}
}
