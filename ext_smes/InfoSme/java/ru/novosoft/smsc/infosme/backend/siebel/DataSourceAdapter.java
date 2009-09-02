package ru.novosoft.smsc.infosme.backend.siebel;

import org.apache.log4j.Category;

import java.util.Map;

/**
 * Created by IntelliJ IDEA.
 * User: bukind
 * Date: 02.09.2009
 * Time: 11:41:26
 * To change this template use File | Settings | File Templates.
 */
public class DataSourceAdapter implements DataSource {

    private static final String siebelTaskPrefix = "siebelTask-";

    private SiebelDataProvider provider_;
    private Category log_;

    DataSourceAdapter( SiebelDataProvider provider, Category log ) {
        this.provider_ = provider;
        this.log_ = log;
    }
    public void saveFinalStates( Map states ) {
        try {
            provider_.updateDeliveryStates(states);
        } catch ( SiebelDataProviderException e ) {
            log_.error("exc to update states:" + e.getMessage());
        }
    }
    public void taskHasFinished( String taskName ) {
        if ( taskName.length() <= siebelTaskPrefix.length() ||
                ! taskName.startsWith(siebelTaskPrefix) ) {
            log_.error("task name " + taskName + " is not siebel");
            return;
        }
        String waveId = taskName.substring(siebelTaskPrefix.length());
        try {
            provider_.setTaskStatus(waveId,SiebelTask.Status.PROCESSED);
            if ( log_.isInfoEnabled() ) {
                log_.info("wave " + waveId + " is processed");
            }
        } catch ( SiebelDataProviderException e ) {
            log_.error("cannot set wave " + waveId + " status: " + e.getMessage());
        }
    }
}
