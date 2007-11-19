package ru.sibinco.smpp.ub_sme.productivity;


public class ProductivityControlObject implements ProductivityControllable {

    private long count;
    private boolean counterEnabled;
    private long counterStartTime;
    private String name;

    public ProductivityControlObject(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public long getEventsCount() {
        return count;
    }

    public synchronized void count() {
        count++;
    }

    public void setEventsCounterEnabled(boolean enabled) {
        counterEnabled = enabled;
        if (counterEnabled) {
            counterStartTime = System.currentTimeMillis();
        }
    }

    public synchronized void resetEventsCounter() {
        count = 0;
        counterStartTime = System.currentTimeMillis();
    }

    public long getCounterStartTime() {
        return counterStartTime;
    }

}
