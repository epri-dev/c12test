// File reader.cpp
//
// Universal configurable reader and link checker

#include <MCORE/MCOREExtern.h>
#include <MCOM/MCOM.h>
#include "C12Tables.h"
#include "C12Meter.h"
#include "Setup.h"
#include <fstream>
#include <iostream>

unsigned failures = 0;


int main(int argc, char *argv[])
{
    Setup setup;
    if (!setup.Initialize(argc, argv))
        return EXIT_FAILURE;

    MProtocol *proto = setup.GetProtocol();
    M_ASSERT(proto != nullptr); // ensured by successful return from Initialize

    MProtocolC12 *protoC12 = M_DYNAMIC_CAST(MProtocolC12, proto);
    if (protoC12 != nullptr)
        protoC12->SetEndSessionOnApplicationLayerError(false);   // this is the only property to override

    std::cout << "Entering test loop. Press Ctrl-C to interrupt." << std::endl;
    class Meter meter;
    try {
        meter.Communicate(*proto, setup.GetTableNames());
        meter.GetResults(*proto, setup.GetTableNames());
    }
    catch(MEOperationCancelled &) {
        std::cout << "Test loop is cancelled with Ctrl-C." << std::endl;
    }
    catch(MException & ex) {
        std::cerr << "### Error: " << ex.AsString() << std::endl;
        ++failures;
    }
    proto->Disconnect();        // never throws
    std::cout << "Errors: " << failures
        << ", retries: " << proto->GetCountLinkLayerPacketsRetried()
        << '\n';
}
