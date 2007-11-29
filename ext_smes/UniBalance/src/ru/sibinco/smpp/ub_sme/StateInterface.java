package ru.sibinco.smpp.ub_sme;

/**
 * Created by Serge Lugovoy
 * Date: Nov 22, 2007
 * Time: 3:32:27 PM
 */
public interface StateInterface {
    public void expire();

    public void startProcessing();

    public void closeProcessing();
}