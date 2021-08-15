// c12test.cpp

#include <MCORE/MCOREExtern.h>
#include <MCOM/MCOM.h>
#include "C12Tables.h"
#include "C12Meter.h"
#include "Setup.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

static bool ReadMeter(Meter& meter, MProtocol* proto, std::vector<std::string> tblvec, unsigned& failures) {
    bool done{false};
    try {
        meter.Communicate(*proto, tblvec);
        meter.GetResults(*proto, tblvec);
    }
    catch(MEOperationCancelled &) {
        std::cout << "Test loop is cancelled with Ctrl-C.\n";
        done = true;
    }
    catch(MException & ex) {
        std::cerr << "### Error: " << ex.AsString() << '\n';
        ++failures;
    }
    proto->Disconnect();        // never throws
    return done;
}

/* 
 * Given a string containing table numbers such as "{ 0 1 20 21 }",
 * returns a vector containing each numeric argument, prepended 
 * with the passed prefix.  If the prefix is "ST" in this case,
 * the returned vector would be "ST0, ST1, ST20, ST21"
 */
static std::vector<std::string> prefixTables(const std::string &tableNumbers, const std::string& prefix) {
    std::stringstream ss{tableNumbers};
    std::string tbl;
    std::vector<std::string> tables;
    while (ss >> tbl) {
        if (std::isdigit(tbl[0])) {
            tables.push_back(prefix + tbl);
        }
    }
    return tables;
}

int main(int argc, char *argv[])
{
    unsigned failures = 0;
    Setup setup;
    if (!setup.Initialize(argc, argv))
        return EXIT_FAILURE;

    MProtocol *proto = setup.GetProtocol();
    M_ASSERT(proto != nullptr); // ensured by successful return from Initialize

    MProtocolC12 *protoC12 = M_DYNAMIC_CAST(MProtocolC12, proto);
    if (protoC12 != nullptr)
        protoC12->SetEndSessionOnApplicationLayerError(true);   // this is the only property to override

    std::cout << "Entering test loop. Press Ctrl-C to interrupt.\n";
    class Meter meter;
    auto tables{setup.GetTableNames()};
    if (setup.GetFullAutoFlag()) {
        if (ReadMeter(meter, proto, std::vector<std::string>{"ST0"}, failures))
            return EXIT_FAILURE;
        tables = prefixTables(meter.evaluateAsString("GEN_CONFIG_TBL.STD_TBLS_USED"), "ST");
        auto mt = prefixTables(meter.evaluateAsString("GEN_CONFIG_TBL.MFG_TBLS_USED"), "MT");
        std::move(mt.begin(), mt.end(), std::back_inserter(tables));
    }
    if (setup.GetSingleFlag()) {
        for (const auto& tbl : tables) {
            std::vector<std::string> tblvec{tbl};
            if (ReadMeter(meter, proto, tblvec, failures))
                break;
        }
    } else {
        ReadMeter(meter, proto, tables, failures);
    }
    std::cout << "Errors: " << failures
        << ", retries: " << proto->GetCountLinkLayerPacketsRetried()
        << '\n';
}
