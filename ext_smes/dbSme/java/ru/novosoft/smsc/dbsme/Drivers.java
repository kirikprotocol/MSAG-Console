package ru.novosoft.smsc.dbsme;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.Iterator;
import java.security.Principal;

/**
 * Created by igork
 * Date: Jul 11, 2003
 * Time: 1:54:30 PM
 */
public class Drivers extends DbsmeBean {
	private String mbCancel = null;
	private String mbDone = null;
	private String dsdriver_new_type = null;
	private String dsdriver_new_loadup = null;
	public static final String prefix = "StartupLoader.DataSourceDrivers";

	public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal, HttpServletRequest request)
	{
		int result = super.process(appContext, errors, loginedPrincipal);
		if (result != RESULT_OK)
			return result;

		if (mbCancel != null)
			return RESULT_DONE;
		if (mbDone != null)
			return save(request);

		return result;
	}

	private int save(HttpServletRequest request)
	{
		config.removeSection(prefix);
		for (Iterator i = request.getParameterMap().keySet().iterator(); i.hasNext();) {
			String paramName = (String) i.next();
			if (paramName.startsWith(prefix + '.')) {
				final Object value = request.getParameterMap().get(paramName);
				if (value instanceof String)
				{
					config.setString(paramName, (String) value);
				} else if (value instanceof String[])
				{
					String valueS = "";
					for (int j = 0; j < ((String[]) value).length; j++) {
						String valPart = ((String[]) value)[j];
						valueS += valPart;
					}
					config.setString(paramName, valueS);
				}
			}
		}

		if (dsdriver_new_type != null && dsdriver_new_type.length() > 0 && dsdriver_new_loadup != null && dsdriver_new_loadup.length() > 0)
		{
			String newDriverPrefix = prefix + '.' + StringEncoderDecoder.encodeDot(dsdriver_new_type);
			config.setString(newDriverPrefix + ".type", dsdriver_new_type);
			config.setString(newDriverPrefix + ".loadup", dsdriver_new_loadup);
		}

		try {
			config.save();
		} catch (Exception e) {
			logger.error("Couldn't save temporary config, nested: " + e.getMessage(), e);
			return error(DBSmeErrors.error.couldntSaveTempConfig, e);
		}

      DbSmeContext.getInstance(appContext).setConfigChanged(true);

		return RESULT_DONE;
	}

	public String getMbCancel()
	{
		return mbCancel;
	}

	public void setMbCancel(String mbCancel)
	{
		this.mbCancel = mbCancel;
	}

	public String getMbDone()
	{
		return mbDone;
	}

	public void setMbDone(String mbDone)
	{
		this.mbDone = mbDone;
	}

	public String getDsdriver_new_type()
	{
		return dsdriver_new_type;
	}

	public void setDsdriver_new_type(String dsdriver_new_type)
	{
		this.dsdriver_new_type = dsdriver_new_type;
	}

	public String getDsdriver_new_loadup()
	{
		return dsdriver_new_loadup;
	}

	public void setDsdriver_new_loadup(String dsdriver_new_loadup)
	{
		this.dsdriver_new_loadup = dsdriver_new_loadup;
	}
}
