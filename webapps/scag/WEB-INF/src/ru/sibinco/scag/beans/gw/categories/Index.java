/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.beans.gw.categories;

import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.TabledBeanImpl;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * The <code>Index</code> class represents
 * <p><p/>
 * Date: 27.10.2005
 * Time: 16:10:06
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Index  extends TabledBeanImpl implements TabledBean
{
  protected Collection getDataSource()
  {
    return appContext.getCategoryManager().getCategories().values();
  }

  protected void delete() throws SCAGJspException
  {
    final List toRemove = new ArrayList(checked.length);
    for (int i = 0; i < checked.length; i++) {
      final String categoryIdStr = checked[i];
      final Long categoryId = Long.decode(categoryIdStr);
      toRemove.add(categoryId);
    }
    appContext.getCategoryManager().getCategories().keySet().removeAll(toRemove);
  }
}