#include <stdexcept>

class failure : public std::exception
{
public:
    virtual const char* what() const throw () { return "failure"; }
};


int fun1( int input )
{
    if ( input > 30 ) throw failure();
    return input + 5;
}

int main()
{
    bool fail = false;
    try {
        int input = 0;
        while (input < 1000) {
            // std::cout << "input:" << input << std::endl;
            input = fun1(input);
        }
    } catch ( std::exception& e ) {
        if ( e.what() ) fail = true;
        // std::cout << "exception:" << e.what() << std::endl;
    }
    return 0;
}
