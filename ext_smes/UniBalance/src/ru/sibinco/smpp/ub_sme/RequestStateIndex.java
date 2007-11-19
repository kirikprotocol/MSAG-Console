package ru.sibinco.smpp.ub_sme;

class RequestStateIndex {
    String abonent = null;

    public RequestStateIndex(String abonent) {
        this.abonent = abonent;
    }

    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof State)) return false;

        final State state = (State) o;

        if (abonent != null ? !abonent.equals(state.getSourceMessage().getSourceAddress()) : state.getSourceMessage().getSourceAddress() != null)
            return false;

        return true;
    }

    public int hashCode() {
        return (abonent != null ? abonent.hashCode() : 0);
    }
}
