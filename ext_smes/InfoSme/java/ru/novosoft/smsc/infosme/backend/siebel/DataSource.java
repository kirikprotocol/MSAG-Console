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
        String smppStatus;

        FinalStateItem( String state, String userData, String smppStatus ) {
            this.state = state;
            this.userData = userData;
            this.smppStatus = smppStatus;
        }
    }

    /// set final states for messages
    public void saveFinalStates( Collection states);
    public void taskHasFinished( String taskName );
}
