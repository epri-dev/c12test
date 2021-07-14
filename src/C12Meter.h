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

C12::Table MakeST0(std::string tabledata);
C12::Table MakeST1(std::string tabledata);
C12::Table MakeST2(std::string tabledata);
C12::Table MakeST3(std::string tabledata);
C12::Table MakeST5(std::string tabledata);
C12::Table MakeST6(std::string tabledata);
C12::Table MakeST20(std::string tabledata);
C12::Table MakeST21(std::string tabledata);
C12::Table MakeST40(std::string tabledata);
C12::Table MakeST41(std::string tabledata);
C12::Table MakeST50(std::string tabledata);
C12::Table MakeST51(std::string tabledata);
C12::Table MakeST52(std::string tabledata);
C12::Table MakeST55(std::string tabledata);
C12::Table MakeST56(std::string tabledata);
C12::Table MakeST60(std::string tabledata);
C12::Table MakeST61(std::string tabledata);
C12::Table MakeST70(std::string tabledata);
C12::Table MakeST71(std::string tabledata);
C12::Table MakeST72(std::string tabledata);
C12::Table MakeST73(std::string tabledata);
#endif // C12METER_H
