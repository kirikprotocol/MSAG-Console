package ru.sibinco.scag.beans.gw.providers;

import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.sme.ProviderManager;
import ru.sibinco.scag.beans.DoneException;
import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.util.Map;


/**
 * Created by igork Date: 22.03.2004 Time: 20:04:20
 */
public class Edit extends EditBean {
    private long id = -1;
    private String name;

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {
        super.process(request, response);
    }

    protected void load(final String loadId) throws SCAGJspException {
        final Map providers = appContext.getProviderManager().getProviders();
        final Long longLoadId = Long.decode(loadId);
        if (!providers.containsKey(longLoadId))
            throw new SCAGJspException(Constants.errors.providers.PROVIDER_NOT_FOUND, loadId);

        final Provider info = (Provider) providers.get(longLoadId);
        this.id = info.getId();
        this.name = info.getName();
    }

    protected void save() throws SCAGJspException {
        final ProviderManager providerManager = appContext.getProviderManager();
        if (isAdd()) {
            providerManager.createProvider(name);
        } else {
            providerManager.setProviderName(id, name);
        }
        appContext.getProviderManager().store();

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
