#ifndef C12METER_H
#define C12METER_H
#include <MCORE/MCOREExtern.h>
#include <MCOM/MCOM.h>
#include "C12Tables.h"

class Meter {
public:
    void Communicate(MProtocol& proto, const MStdStringVector& tables);
    void GetResults(MProtocol& proto, const MStdStringVector& tables);
    long evaluate(const std::string& expression);
    std::string evaluateAsString(const std::string& expression) const;
    void interpret(int itemInt, MProtocol& proto, int count);
private:
    std::vector<C12::Table> table = {};
};

#endif // C12METER_H
