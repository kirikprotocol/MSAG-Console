/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.beans.gw.categories;

import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.sme.Category;
import ru.sibinco.scag.backend.sme.CategoryManager;
import ru.sibinco.scag.beans.DoneException;
import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.lib.backend.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.Map;
import java.io.IOException;

/**
 * The <code>Edit</code> class represents
 * <p><p/>
 * Date: 27.10.2005
 * Time: 16:09:56
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Edit extends EditBean {
    private long id = -1;
    private String name;

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {
        super.process(request, response);
    }

    protected void load(final String loadId) throws SCAGJspException {
        final Map categories = appContext.getCategoryManager().getCategories();
        final Long longLoadId = Long.decode(loadId);
        if (!categories.containsKey(longLoadId))
            throw new SCAGJspException(Constants.errors.categories.CATEGORY_NOT_FOUND, loadId);

        final Category info = (Category) categories.get(longLoadId);
        this.id = info.getId();
        this.name = info.getName();
    }

    protected void save() throws SCAGJspException {
        final CategoryManager categoryManager = appContext.getCategoryManager();

        if (isAdd()) {
            categoryManager.createCategory(name);
        } else {
            categoryManager.setCategoryName(id, name);
        }
        appContext.getCategoryManager().store();
        try {
            appContext.getIdsConfig().save();
        } catch (Config.WrongParamTypeException e) {
            logger.debug("Couldn't save config", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_SAVE_CONFIG, e);
        } catch (IOException e) {
            logger.debug("Couldn't save config", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_SAVE_CONFIG, e);
        }

        throw new DoneException();
    }

    public String getId() {
        return -1 == id ? null : String.valueOf(id);
    }

    public void setId(final String id) {
        this.id = Long.decode(id).longValue();
    }

    public String getName() {
        return name;
    }

    public void setName(final String name) {
        this.name = name;
    }
}
