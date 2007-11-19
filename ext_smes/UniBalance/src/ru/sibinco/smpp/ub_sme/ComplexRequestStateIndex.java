package ru.sibinco.smpp.ub_sme;


public class ComplexRequestStateIndex {
    private String sourceAddress;
    private String destAddress;


    public ComplexRequestStateIndex(String sourceAddress, String destAddress) {
        this.sourceAddress = sourceAddress;
        this.destAddress = destAddress;
    }

    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof State)) return false;

        final State state = (State) o;

        if (sourceAddress != null && !sourceAddress.equals(state.getSourceMessage().getSourceAddress())) {
            return false;
        }
        if (destAddress != null && !destAddress.equals(state.getSourceMessage().getDestinationAddress())) {
            return false;
        }
        return true;

    }

}
