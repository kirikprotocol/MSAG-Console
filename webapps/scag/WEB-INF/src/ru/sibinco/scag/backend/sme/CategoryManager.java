/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.sme;

import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.beans.SCAGJspException;

import java.util.Map;
import java.util.Collections;
import java.util.TreeMap;
import java.util.Collection;
import java.util.Iterator;
import java.io.PrintWriter;
import java.io.FileWriter;
import java.io.IOException;

/**
 * The <code>CategoryManager</code> class represents
 * <p><p/>
 * Date: 24.10.2005
 * Time: 16:55:03
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class CategoryManager {

    private final Map categories = Collections.synchronizedMap(new TreeMap());
    private long lastUsedId;
    private final Config idsConfig;
    private static final String SECTION_NAME = "categories";
    private static final String PARAM_NAME_LAST_USED_ID = "last used category id";


    public CategoryManager(Config idsConfig) throws Config.WrongParamTypeException, Config.ParamNotFoundException, NumberFormatException {

        lastUsedId = idsConfig.getInt(SECTION_NAME + '.' + PARAM_NAME_LAST_USED_ID);
        this.idsConfig = idsConfig;
        final Collection categoryIds = idsConfig.getSectionChildShortParamsNames(SECTION_NAME);
        for (Iterator i = categoryIds.iterator(); i.hasNext();) {
            final String categoryIdStr = (String) i.next();
            if (!PARAM_NAME_LAST_USED_ID.equalsIgnoreCase(categoryIdStr)) {
                final Long categoryId = Long.decode(categoryIdStr);
                final Category category = createCategory(categoryId.longValue(), idsConfig.getString(SECTION_NAME + '.' + categoryIdStr));
                categories.put(categoryId, category);
            }
        }
    }

    public synchronized Map getCategories() {
        return categories;
    }

    private synchronized Category createCategory(final long id, final String name) {
        final Category category = new Category(id, name);
        categories.put(new Long(category.getId()), category);
        return category;
    }

    public synchronized Category createCategory(final String name) {
        return createCategory(++lastUsedId, name);
    }

    public synchronized void setCategoryName(final long id, final String name) throws NullPointerException {
        final Category category = (Category) categories.get(new Long(id));
        if (null == category)
            throw new NullPointerException("Category \"" + id + "\" not found.");
        category.setName(name);
    }

    public synchronized void store(final Config idsConfig) {
        idsConfig.removeSection(SECTION_NAME);
        idsConfig.setInt(SECTION_NAME + '.' + PARAM_NAME_LAST_USED_ID, lastUsedId);
        for (Iterator i = categories.values().iterator(); i.hasNext();) {
            final Category category = (Category) i.next();
            idsConfig.setString(SECTION_NAME + "." + category.getId(), category.getName());
        }
    }

    public void store(){
        store(idsConfig);
    }

}


