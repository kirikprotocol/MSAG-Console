/*
 * Created by igork
 * Date: 07.10.2002
 * Time: 22:30:04
 */
package ru.novosoft.smsc.util.config;

import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.io.IOException;
import java.io.OutputStream;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

public class SaveableConfigTree
{
	private Map sections = new HashMap();
	private Map params = new HashMap();

	public SaveableConfigTree(Config config)
	{
		for (Iterator i = config.params.keySet().iterator(); i.hasNext();)
		{
		  String name = (String) i.next();
		  this.putParameter(name, config.params.get(name));
		}
	}

	protected SaveableConfigTree()
	{
	}

	public void putParameter(String name, Object value)
	{
		int dotpos = name.indexOf('.');
		if (dotpos < 0)
		{
			params.put(name, value);
		}
		else if (dotpos == 0)
		{
			putParameter(name.substring(1), value);
		}
		else
		{
			SaveableConfigTree sec = (SaveableConfigTree) sections.get(name.substring(0, dotpos));
			if (sec == null)
			{
				sec = new SaveableConfigTree();
				sections.put(name.substring(0, dotpos), sec);
			}
			sec.putParameter(name.substring(dotpos + 1), value);
		}
	}

	public void write(OutputStream out, String prefix)
			  throws ConfigManager.WrongParamTypeException, IOException
	{
		writeParams(out, prefix, params);
		writeSections(out, prefix, sections);
	}

	private void writeParams(OutputStream out, String prefix, Map parameters)
			  throws ConfigManager.WrongParamTypeException, IOException
	{
		for (Iterator i = parameters.keySet().iterator(); i.hasNext();)
		{
			String paramName = (String) i.next();
			Object paramValue = parameters.get(paramName);
			out.write((prefix + "<param name=\"" + StringEncoderDecoder.encode(paramName) + "\" type=\"").getBytes());
			if (paramValue instanceof String)
			{
				out.write(("string\">" + StringEncoderDecoder.encode((String) paramValue) + "</param>\n").getBytes());
			}
			else if (paramValue instanceof Integer)
			{
				out.write(("int\">" + StringEncoderDecoder.encode(String.valueOf(((Integer) paramValue).longValue())) + "</param>\n").getBytes());
			}
			else if (paramValue instanceof Long)
			{
				out.write(("int\">" + StringEncoderDecoder.encode(String.valueOf(((Long) paramValue).longValue())) + "</param>\n").getBytes());
			}
			else if (paramValue instanceof Boolean)
			{
				out.write(("bool\">" + StringEncoderDecoder.encode(String.valueOf(((Boolean) paramValue).booleanValue())) + "</param>\n").getBytes());
			}
			else
			{
				throw new ConfigManager.WrongParamTypeException("unknown type of parameter " + paramName);
			}
		}
	}

	private void writeSections(OutputStream out, String prefix, Map secs)
			  throws IOException, ConfigManager.WrongParamTypeException
	{
		for (Iterator i = secs.keySet().iterator(); i.hasNext();)
		{
			String secName = (String) i.next();
			SaveableConfigTree childs = (SaveableConfigTree) secs.get(secName);
			out.write((prefix + "<section name=\"" + StringEncoderDecoder.encode(secName) + "\">\n").getBytes());
			childs.write(out, prefix + "  ");
			out.write((prefix + "</section>\n").getBytes());
		}
	}
}
