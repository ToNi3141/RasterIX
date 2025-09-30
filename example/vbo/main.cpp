#include "Runner.hpp"
#include "VboExample.hpp"

int main()
{
    static Runner<VboExample> r;
    r.execute();
    return 0;
}
