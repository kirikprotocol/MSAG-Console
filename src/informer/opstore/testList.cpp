#include <list>
#include <stdio.h>

typedef std::list<int> intlist;

void showend(const char* where, intlist& l)
{
    printf("%s: begin=%p end=%p\n",where,
           reinterpret_cast<const void*>(&*l.begin()),
           reinterpret_cast<const void*>(&*l.end()));
}

template<class iter>
void showiter(const char* what,iter it)
{
    printf("iter %s: %p\n",what,reinterpret_cast<const void*>(&*it));
}


int main()
{
    intlist l;
    showend("created",l);

    l.insert(l.end(),10);
    showend("inserted one elt at end",l);

    l.clear();
    showend("cleared",l);

    intlist other;
    other.insert(other.end(),20);
    other.insert(other.end(),30);
    l.splice(l.begin(),other);
    showend("spliced list at begin",l);

    l.clear();

    other.insert(other.end(),20);
    other.insert(other.end(),30);
    l.splice(l.end(),other);
    showend("spliced list at end",l);

    {
        intlist::iterator x = l.begin();
        showiter("begin",x);
        showiter("begin+1",++x);
        x = l.end();
        showiter("end",x);
        showiter("end-1",--x);
        intlist::reverse_iterator y = l.rbegin();
        showiter("rbegin",y);
        showiter("rbegin+1",++y);
        y = l.rend();
        showiter("rend",y);
        showiter("rend-1",--y);
        y = l.rbegin();
        showiter("rbegin.base",y.base());
        showiter("rbegin+1.base",(++y).base());

        l.erase(y.base());
        showend("erase once before end",l);
    }

    l.swap(other);
    showend("swapped",l);
    return 0;
}
