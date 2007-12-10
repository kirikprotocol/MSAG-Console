package ru.sibinco.smpp.ub_sme;

import ru.sibinco.smpp.ub_sme.mg.MGState;
import org.apache.log4j.Category;

/**
 * User: pasha
 * Date: 27.11.2007
 * Time: 19:17:09
 */
public class ProfileStateProcessor implements Runnable {
        private static org.apache.log4j.Category logger =
            Category.getInstance(ProfileStateProcessor.class);
    protected MGState state;

    public ProfileStateProcessor(MGState state) {
        this.state = state;
    }

    public void run() {
        
        state.setProfState(MGState.PROF_WAIT);
        String encoding = ProfileAbonentManager.getProfileAbonentLang().getLangProfile(state.getAbonentRequest().getSourceAddress());
        if(logger.isDebugEnabled()){
            logger.debug("Abonent "+state.getAbonentRequest().getSourceAddress() +" encoding:"+encoding);
        }
        if (null == encoding) {
            state.setProfState(MGState.PROF_ERR);
        } else {
            state.setEncoding(encoding);
            state.setProfState(MGState.PROF_OK);
        }
        state.closeProcessing();
    }

}
