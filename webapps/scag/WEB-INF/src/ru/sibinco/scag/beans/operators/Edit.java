/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans.operators;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.operators.Operator;
import ru.sibinco.scag.backend.operators.OperatorManager;
import ru.sibinco.scag.beans.DoneException;
import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

/**
 * The <code>Edit</code> class represents
 * <p><p/>
 * Date: 02.03.2006
 * Time: 13:52:12
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Edit extends EditBean {

    private long id = -1;
    private String name;
    private String description;
    private String[] srcMasks = new String[0];


    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {

        super.process(request, response);
    }

    protected void load(final String loadId) throws SCAGJspException {

        final Map operators = appContext.getOperatorManager().getOperators();
        final Long longLoadId = Long.decode(loadId);
        if (!operators.containsKey(longLoadId))
            throw new SCAGJspException(Constants.errors.operators.OPERATOR_NOT_FOUND, loadId);

        final Operator operator = (Operator) operators.get(longLoadId);
        this.id = operator.getId().longValue();
        this.name = operator.getName();
        this.description = operator.getDescription();

        final List maskList = new ArrayList();
        for (Iterator i = operator.getMasks().values().iterator(); i.hasNext();) {
            final Mask mask = (Mask) i.next();
            maskList.add(mask.getMask());

        }
        srcMasks = (String[]) maskList.toArray(new String[0]);
        maskList.clear();
    }

    protected void save() throws SCAGJspException {
        final OperatorManager operatorManager = appContext.getOperatorManager();
        if (isAdd()) {
            try {
                id = operatorManager.createOperator(name, description, srcMasks);
            } catch (SibincoException e) {
                logger.debug("Couldn't create new operator", e);
                throw new SCAGJspException(Constants.errors.operators.COULD_NOT_CREATE_OPERATOR, e);
            }
        } else {
            try {
                operatorManager.updateOperator(id, name, description, srcMasks);
            } catch (SibincoException e) {
                logger.debug("Couldn't update operator", e);
                throw new SCAGJspException(Constants.errors.operators.COULD_NOT_UPDATE_OPERATOR, e);
            }
        }
        try {
            appContext.getOperatorManager().store();
        } catch (IOException e) {
            logger.debug("Couldn't save config", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_SAVE_CONFIG, e);
        }

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

    public void setName(String name) {
        this.name = name;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public String[] getSrcMasks() {
        return srcMasks;
    }

    public void setSrcMasks(String[] srcMasks) {
        this.srcMasks = srcMasks;
    }
}
