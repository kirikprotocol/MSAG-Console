package ru.sibinco.smpp.ub_sme;

import ru.sibinco.smpp.ub_sme.mg.MGState;

/**
 * User: pasha
 * Date: 27.11.2007
 * Time: 19:17:09
 */
public class ProfileStateProcessor implements Runnable {
    protected MGState state;

    public ProfileStateProcessor(MGState state) {
        this.state = state;
    }

    public void run() {
        state.setProfState(MGState.PROF_WAIT);
        String encoding = ProfileAbonentManager.getProfileAbonentLang().getLangProfile(state.getAbonentRequest().getSourceAddress());
        if (null == encoding) {
            state.setProfState(MGState.PROF_ERR);
        } else {
            state.setEncoding(encoding);
            state.setProfState(MGState.PROF_OK);
        }
    }
}
