package ru.sibinco.smpp.ub_sme;

import ru.sibinco.smpp.ub_sme.mg.MGState;

import java.io.UnsupportedEncodingException;

/**
 * User: pasha
 * Date: 27.11.2007
 * Time: 18:18:14
 */
public class BannerRequestStateProcessor implements Runnable {
    private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(BannerRequestStateProcessor.class);
    protected MGState state;

    public BannerRequestStateProcessor(MGState state) {
        this.state = state;
    }

    public void run() {
        state.setBannerState(MGState.BE_RESP_WAIT);
        String banner = Sme.getSmeEngine().getBanner(state);
        if(logger.isDebugEnabled()){
            logger.debug("Got banner:"+banner);
        }
        //TODO delete it
        banner ="banner";
        if (null == banner) {
            state.setBannerState(MGState.BE_RESP_ERR);
        } else {
            state.setBanner(banner);
            state.setBannerState(MGState.BE_RESP_OK);
        }
    }
}
