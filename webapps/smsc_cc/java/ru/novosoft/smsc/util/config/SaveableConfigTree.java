package ru.novosoft.smsc.util.config;

import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.*;

/**
 * Вспомогательный класс, преобразующий конфиг в дерево.
 */
class SaveableConfigTree {

  private Map<String, SaveableConfigTree> sections = new HashMap<String, SaveableConfigTree>();
  private Map<String, Object> params = new HashMap<String, Object>();

  public SaveableConfigTree(Config config) {
    for (String name : config.params.keySet()) {
      this.putParameter(name, config.params.get(name));
    }
  }

  protected SaveableConfigTree() {
  }

  private void putParameter(String name, Object value) {
    int dotpos = name.indexOf('.');
    if (dotpos < 0) {
      params.put(name, value);
    } else if (dotpos == 0) {
      putParameter(name.substring(1), value);
    } else {
      SaveableConfigTree sec = sections.get(name.substring(0, dotpos));
      if (sec == null) {
        sec = new SaveableConfigTree();
        sections.put(name.substring(0, dotpos), sec);
      }
      sec.putParameter(name.substring(dotpos + 1), value);
    }
  }

  public void write(PrintWriter out, String prefix) throws Config.WrongParamTypeException, IOException {
    writeParams(out, prefix, params);
    if (sections.size() > 0)
      out.println();
    writeSections(out, prefix, sections);
  }

  private void writeParams(PrintWriter out, String prefix, Map<String, Object> parameters) throws Config.WrongParamTypeException {
    List<String> paramNames = new ArrayList<String>(parameters.keySet());
    Collections.sort(paramNames);
    for (String paramName : paramNames) {
      Object paramValue = parameters.get(paramName);
      out.print((prefix + "<param name=\"" + StringEncoderDecoder.encode(paramName) + "\" type=\""));
      if (paramValue instanceof String) {
        out.println(("string\">" + StringEncoderDecoder.encode((String) paramValue) + "</param>"));
      } else if (paramValue instanceof Integer) {
        out.println(("int\">" + StringEncoderDecoder.encode(String.valueOf(((Integer) paramValue).longValue())) + "</param>"));
      } else if (paramValue instanceof Long) {
        out.println(("int\">" + StringEncoderDecoder.encode(String.valueOf(((Long) paramValue).longValue())) + "</param>"));
      } else if (paramValue instanceof Boolean) {
        out.println(("bool\">" + StringEncoderDecoder.encode(String.valueOf(((Boolean) paramValue).booleanValue())) + "</param>"));
      } else {
        throw new Config.WrongParamTypeException("unknown type of parameter " + paramName);
      }
    }
  }

  private void writeSections(PrintWriter out, String prefix, Map<String, SaveableConfigTree> secs) throws IOException, Config.WrongParamTypeException {
    List<String> secNames = new ArrayList<String>(secs.keySet());
    Collections.sort(secNames);
    for (Iterator i = secNames.iterator(); i.hasNext();) {
      String secName = (String) i.next();
      SaveableConfigTree childs = secs.get(secName);
      out.println((prefix + "<section name=\"" + StringEncoderDecoder.encode(secName) + "\">"));
      childs.write(out, prefix + "  ");
      out.println((prefix + "</section>"));
      if (i.hasNext())
        out.println();
    }
  }
}
