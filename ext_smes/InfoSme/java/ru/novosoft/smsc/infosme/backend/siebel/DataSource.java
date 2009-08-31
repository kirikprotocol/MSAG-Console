package ru.novosoft.smsc.infosme.backend.siebel;

import java.util.Collection;

/**
 * Created by IntelliJ IDEA.
 * User: bukind
 * Date: 31.08.2009
 * Time: 13:48:15
 * To change this template use File | Settings | File Templates.
 */
public interface DataSource {

    public class FinalStateItem {
        String state;
        String userData;

        FinalStateItem( String state, String userData ) {
            this.state = state;
            this.userData = userData;
        }
    }

    /// set final states for messages
    public void saveFinalStates( Collection states);

}
