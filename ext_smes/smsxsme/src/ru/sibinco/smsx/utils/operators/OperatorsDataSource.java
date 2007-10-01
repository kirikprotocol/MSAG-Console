package ru.sibinco.smsx.utils.operators;

import com.eyeline.sme.utils.config.xml.XmlConfig;
import com.eyeline.sme.utils.config.xml.XmlConfigSection;
import com.eyeline.sme.utils.ds.DataSourceException;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;

/**
 * User: artem
 * Date: 09.07.2007
 */

class OperatorsDataSource {

  private final String file;

  OperatorsDataSource(String file) {
    this.file = file;
  }

  public Collection getOperators() throws DataSourceException {
    final XmlConfig cfg = new XmlConfig();

    final Collection result = new LinkedList();

    InputStream is = null;

    try {
      is = new FileInputStream(file);
      cfg.load(is);

      XmlConfigSection sec;
      Operator oper;
      for (Iterator iter = cfg.sections(); iter.hasNext();) {
        sec = (XmlConfigSection)iter.next();
        oper = new Operator(sec.getName());

        if (sec.getParam("masks") != null) {
          final String[] masks = sec.getParam("masks").getStringList(",");
          for (int i=0; i<masks.length; i++)
            oper.addMask(masks[i]);
        }

        result.add(oper);
      }

    } catch (Throwable e) {
      throw new DataSourceException(e);
    } finally {
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {
      }
    }

    return result;
  }
}
