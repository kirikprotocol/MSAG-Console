/*
 * Created by igork
 * Date: 07.10.2002
 * Time: 22:30:04
 */
package ru.novosoft.smsc.util.config;

import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.io.*;
import java.util.*;

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

	public void write(OutputStream out, String prefix) throws Config.WrongParamTypeException, IOException
	{
		write(new PrintWriter(new OutputStreamWriter(out)), prefix);
	}

	public void write(PrintWriter out, String prefix) throws Config.WrongParamTypeException, IOException
	{
		writeParams(out, prefix, params);
		writeSections(out, prefix, sections);
	}

	private void writeParams(PrintWriter out, String prefix, Map parameters) throws Config.WrongParamTypeException
	{
		List paramNames = new ArrayList(parameters.keySet());
		Collections.sort(paramNames, new Comparator()
		{
			public int compare(Object o1, Object o2)
			{
				String s1 = (String) o1;
				String s2 = (String) o2;
				return s1.compareTo(s2);
			}
		});
		for (Iterator i = paramNames.iterator(); i.hasNext();)
		{
			String paramName = (String) i.next();
			Object paramValue = parameters.get(paramName);
			out.print((prefix + "<param name=\"" + StringEncoderDecoder.encode(paramName) + "\" type=\""));
			if (paramValue instanceof String)
			{
				out.println(("string\">" + StringEncoderDecoder.encode((String) paramValue) + "</param>"));
			}
			else if (paramValue instanceof Integer)
			{
				out.println(("int\">" + StringEncoderDecoder.encode(String.valueOf(((Integer) paramValue).longValue())) + "</param>"));
			}
			else if (paramValue instanceof Long)
			{
				out.println(("int\">" + StringEncoderDecoder.encode(String.valueOf(((Long) paramValue).longValue())) + "</param>"));
			}
			else if (paramValue instanceof Boolean)
			{
				out.println(("bool\">" + StringEncoderDecoder.encode(String.valueOf(((Boolean) paramValue).booleanValue())) + "</param>"));
			}
			else
			{
				throw new Config.WrongParamTypeException("unknown type of parameter " + paramName);
			}
		}
	}

	private void writeSections(PrintWriter out, String prefix, Map secs) throws IOException, Config.WrongParamTypeException
	{
		List secNames = new ArrayList(secs.keySet());
		Collections.sort(secNames, new Comparator()
		{
			public int compare(Object o1, Object o2)
			{
				String s1 = (String) o1;
				String s2 = (String) o2;
				return s1.compareTo(s2);
			}
		});
		for (Iterator i = secNames.iterator(); i.hasNext();)
		{
			String secName = (String) i.next();
			SaveableConfigTree childs = (SaveableConfigTree) secs.get(secName);
			out.println((prefix + "<section name=\"" + StringEncoderDecoder.encode(secName) + "\">"));
			childs.write(out, prefix + "  ");
			out.println((prefix + "</section>\n"));
		}
	}
}
