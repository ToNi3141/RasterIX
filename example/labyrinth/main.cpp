#include "Labyrinth.hpp"
#include "Runner.hpp"

int main()
{
    static Runner<Labyrinth> r;
    r.execute();
    return 0;
}
