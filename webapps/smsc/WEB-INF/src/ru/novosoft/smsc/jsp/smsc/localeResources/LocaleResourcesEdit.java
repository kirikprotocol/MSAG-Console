package ru.novosoft.smsc.jsp.smsc.localeResources;

import org.w3c.dom.*;
import ru.novosoft.smsc.jsp.*;
import ru.novosoft.smsc.util.xml.Utils;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.security.Principal;
import java.util.*;
import java.io.*;

/**
 * Created by igork
 * Date: 12.05.2003
 * Time: 18:39:59
 */
public class LocaleResourcesEdit extends PageBean
{
	private final String SETTINGS_SECTION_NAME = "settings";
	private final String RESOURCES_SECTION_NAME = "resources";
	private String locale = null;
	private boolean initialized = false;
	private String mbSave = null;
	private String mbCancel = null;

	private Section settings = new Section(SETTINGS_SECTION_NAME, null);
	private Section resources = new Section(RESOURCES_SECTION_NAME, null);

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		if (!initialized)
		{
			Document localeDom = null;
			try
			{
				localeDom = Utils.parse(appContext.getResourcesManager().getResource(locale));
			}
			catch (Throwable e)
			{
				logger.error("Couldn't parse locale \"" + locale + "\"", e);
			}
			Element rootElement = localeDom.getDocumentElement();
			NodeList settingsNodeList = rootElement.getElementsByTagName("settings");
			for (int i = 0; i < settingsNodeList.getLength(); i++)
			{
				parseSection(settings, (Element) settingsNodeList.item(i));
			}
			NodeList resourcesNodeList = rootElement.getElementsByTagName("resources");
			for (int i = 0; i < resourcesNodeList.getLength(); i++)
			{
				parseSection(resources, (Element) resourcesNodeList.item(i));
			}
		}
		return result;
	}

	private void parseSection(Section section, Element sectionElem)
	{
		NodeList childNodes = sectionElem.getChildNodes();
		for (int i = 0; i < childNodes.getLength(); i++)
		{
			Node childNode = childNodes.item(i);
			if (childNode.getNodeType() == Node.ELEMENT_NODE)
			{
				Element childElement = (Element) childNode;
				if ("section".equalsIgnoreCase(childElement.getTagName()))
				{
					String subSectionName = childElement.getAttribute("name");
					Section subSection = section.findSection(subSectionName);
					parseSection(subSection, childElement);
				}
				else if ("param".equalsIgnoreCase(childElement.getTagName()))
				{
					String paramName = childElement.getAttribute("name");
					String paramValue = Utils.getNodeText(childElement);
					section.setParam(paramName, paramValue);
				}
			}
		}
	}

	public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal, Map params)
	{
		int result = super.process(appContext, errors, loginedPrincipal);
		if (result != RESULT_OK)
			return result;

		processDataParams(params);
		if (mbCancel != null)
			return RESULT_DONE;
		else if (mbSave != null)
			return save();

		return RESULT_OK;
	}

	private int save()
	{
		try
		{
			File tempFile = File.createTempFile("new_locale_", ".xml.tmp");
			PrintWriter out = new PrintWriter(new FileWriter(tempFile));
			out.println("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>");
			out.println("<!DOCTYPE locale_resources SYSTEM \"file://locale_resources.dtd\">");
			out.println();
			out.println("<locale_resources>");
			out.println("<settings>");
			storeSection(settings, out, "   ");
			out.println("</settings>");
			out.println("<resources>");
			storeSection(resources, out, "   ");
			out.println("</resources>");
			out.println("</locale_resources>");
			out.flush();
			out.close();
			return appContext.getResourcesManager().add(locale, tempFile) ? RESULT_DONE : error(SMSCErrors.error.localeResources.couldntCreateFile);
		}
		catch (IOException e)
		{
         return error(SMSCErrors.error.localeResources.couldntStoreFile, e);
		}
	}

	private void storeSection(Section section, PrintWriter out, String prefix)
	{
		for (Iterator i = section.getSectionNames().iterator(); i.hasNext();)
		{
			String sectionName = (String) i.next();
			out.println(prefix + "<section name=\"" + StringEncoderDecoder.encode(sectionName) + "\">");
			storeSection(section.getSection(sectionName), out, prefix + "   ");
			out.println(prefix + "</section>");
		}
		for (Iterator i = section.getParamNames().iterator(); i.hasNext();)
		{
			String paramName = ((String) i.next()).trim();
			String paramValue = section.getParam(paramName).trim();
			if (paramValue != null && paramValue.length() > 0)
				out.println(prefix + "<param name=\"" + StringEncoderDecoder.encode(paramName) + "\">" + StringEncoderDecoder.encode(paramValue) + "</param>");
			else
				out.println(prefix + "<param name=\"" + StringEncoderDecoder.encode(paramName) + "\"/>");
		}
	}

	private void processDataParams(Map params)
	{
		for (Iterator i = params.keySet().iterator(); i.hasNext();)
		{
			String paramName = (String) i.next();
			Object param = params.get(paramName);
			if (param instanceof String)
				processDataParam(paramName, (String) param);
			else if (param instanceof String[])
			{
				final String[] paramValues = (String[]) param;
				for (int j = 0; j < paramValues.length; j++)
					processDataParam(paramName, paramValues[j]);
			}
			else
			{
				//skip this strange param
				//logger.debug("processDataParams: skip param: \"" + paramName + "\"=\"" + param + "\"");
			}
		}
	}

	private void processDataParam(String fullName, String paramValue)
	{
		int dotPos = fullName.indexOf(Section.NAME_DELIMETER);
		if (dotPos > 0)
		{
			String rootSectionName = fullName.substring(0, dotPos);
			String fullParamName = fullName.substring(dotPos + 1);
			if (rootSectionName.equals(SETTINGS_SECTION_NAME))
				aaa(fullParamName, paramValue, settings);
			else if (rootSectionName.equals(RESOURCES_SECTION_NAME))
				aaa(fullParamName, paramValue, resources);
			else
			{
				//logger.debug("processDataParam: skip param: \"" + fullName + "\"=\"" + paramValue + "\"");
				//skip strange values
			}
		}
		else
		{
			//logger.debug("processDataParam: skip misformatted param: \"" + fullName + "\"=\"" + paramValue + "\"");
			//skip strange values
		}
	}

	private void aaa(String fullParamName, String paramValue, Section rootSection)
	{
		int dotPos = fullParamName.lastIndexOf(Section.NAME_DELIMETER);
		Section section;
		String paramName;
		if (dotPos >= 0)
		{
			section = rootSection.findSection(fullParamName.substring(0, dotPos));
			paramName = fullParamName.substring(dotPos + 1);
		}
		else
		{
			section = rootSection;
			paramName = fullParamName;
		}
		section.setParam(paramName, paramValue);
	}

	public String getLocale()
	{
		return locale;
	}

	public void setLocale(String locale)
	{
		this.locale = locale;
	}

	public Section getSettings()
	{
		return settings;
	}

	public Section getResources()
	{
		return resources;
	}

	public boolean isInitialized()
	{
		return initialized;
	}

	public void setInitialized(boolean initialized)
	{
		this.initialized = initialized;
	}

	public String getMbSave()
	{
		return mbSave;
	}

	public void setMbSave(String mbSave)
	{
		this.mbSave = mbSave;
	}

	public String getMbCancel()
	{
		return mbCancel;
	}

	public void setMbCancel(String mbCancel)
	{
		this.mbCancel = mbCancel;
	}
}
