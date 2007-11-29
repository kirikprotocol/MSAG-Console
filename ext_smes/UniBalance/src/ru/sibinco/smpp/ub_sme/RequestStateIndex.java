package ru.sibinco.smpp.ub_sme;

import ru.sibinco.smpp.ub_sme.RequestState;

class RequestStateIndex {
    String abonent = null;

    public RequestStateIndex(String abonent) {
        this.abonent = abonent;
    }

    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof RequestState)) return false;

        final RequestState state = (RequestState) o;

        if (abonent != null ? !abonent.equals(state.getAbonentRequest().getSourceAddress()) : state.getAbonentRequest().getSourceAddress() != null)
            return false;

        return true;
    }

    public int hashCode() {
        return (abonent != null ? abonent.hashCode() : 0);
    }
}
