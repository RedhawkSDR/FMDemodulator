#include <iostream>
#include "ossie/ossieSupport.h"

#include "FMDemodulator.h"


int main(int argc, char* argv[])
{
    FMDemodulator_i* FMDemodulator_servant;
    Resource_impl::start_component(FMDemodulator_servant, argc, argv);
    return 0;
}
