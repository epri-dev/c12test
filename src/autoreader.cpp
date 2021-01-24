// File reader.cpp
//
// Universal configurable reader and link checker

#include <MCORE/MCOREExtern.h>
#include <MCOM/MCOM.h>
#include <fstream>
#include <signal.h>
#include <array>
#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <iomanip>
#include "Setup.h"


#define SHOW(ostr, item) ostr << #item " = " << item << '\n'
#define SHOWXB(ostr, item) ostr << #item " = 0x" << std::hex << std::setfill('0') << std::setw(2) << (unsigned)item << '\n'
#define SHOWBSET(ostr, bset) ostr << #bset << std::dec << ", size = " << bset.size() << " = { "; \
    for (std::size_t i{0}; i < bset.size(); ++i) if (bset[i]) ostr << i << ' '; \
        ostr << "}\n";
#define SHOWCHARARRAY(ostr, carr) ostr << #carr << " = \""; \
    std::copy(carr.begin(), carr.end(), std::ostream_iterator<char>(ostr, "")); \
        ostr << "\"\n";

std::vector<bool> fetchSet(std::string::iterator& begin, std::size_t size) {
    auto end{begin + size};
    std::vector<bool> retval;
    retval.reserve(size * 8);
    for (auto count{size}; begin < end; ++begin) {
        for (uint8_t mask{1}; mask; mask <<= 1) {
            if (count--)
                retval.push_back(*begin & mask);
            else
                break;
        }
    }
    return retval;
}

struct ST_000_GEN_CONFIG_TBL
{
    uint8_t FORMAT_CONTROL_1;
    uint8_t FORMAT_CONTROL_2;
    uint8_t FORMAT_CONTROL_3;
    std::array<uint8_t, 4> DEVICE_CLASS;
    uint8_t NAMEPLATE_TYPE;
    uint8_t DEFAULT_SET_USED;
    uint8_t MAX_PROC_PARM_LENGTH;
    uint8_t MAX_RESP_DATA_LEN;
    uint8_t STD_VERSION_NO;
    uint8_t STD_REVISION_NO;
    uint8_t DIM_STD_TBLS_USED;
    uint8_t DIM_MFG_TBLS_USED;
    uint8_t DIM_STD_PROC_USED;
    uint8_t DIM_MFG_PROC_USED;
    uint8_t DIM_MFG_STATUS_USED;
    uint8_t NBR_PENDING;
    std::vector<bool> STD_TBLS_USED;
    std::vector<bool> MFG_TBLS_USED;
    std::vector<bool> STD_PROC_USED;
    std::vector<bool> MFG_PROC_USED;
    std::vector<bool> STD_TBLS_WRITE;
    std::vector<bool> MFG_TBLS_WRITE;

    ST_000_GEN_CONFIG_TBL(std::string bytes) {
        if (bytes.size() >= 18) {
            auto here{bytes.begin()};
            FORMAT_CONTROL_1 = *here++;
            FORMAT_CONTROL_2 = *here++;
            FORMAT_CONTROL_3 = *here++;
            std::copy(here, here + DEVICE_CLASS.size(), DEVICE_CLASS.begin());
            here += DEVICE_CLASS.size();
            NAMEPLATE_TYPE = *here++;
            DEFAULT_SET_USED = *here++;
            MAX_PROC_PARM_LENGTH = *here++;
            MAX_RESP_DATA_LEN = *here++;
            STD_VERSION_NO = *here++;
            STD_REVISION_NO = *here++;
            DIM_STD_TBLS_USED = *here++;
            DIM_MFG_TBLS_USED = *here++;
            DIM_STD_PROC_USED = *here++;
            DIM_MFG_PROC_USED = *here++;
            DIM_MFG_STATUS_USED = *here++;
            NBR_PENDING = *here++;
            STD_TBLS_USED = fetchSet(here, DIM_STD_TBLS_USED);
            MFG_TBLS_USED = fetchSet(here, DIM_MFG_TBLS_USED);
            STD_PROC_USED = fetchSet(here, DIM_STD_PROC_USED);
            MFG_PROC_USED = fetchSet(here, DIM_MFG_PROC_USED);
            STD_TBLS_WRITE = fetchSet(here, DIM_STD_TBLS_USED);
            MFG_TBLS_WRITE = fetchSet(here, DIM_MFG_TBLS_USED);
        }
    }

    friend std::ostream& operator<<(std::ostream& out, const ST_000_GEN_CONFIG_TBL& st0) {
        SHOWXB(out, st0.FORMAT_CONTROL_1);
        SHOWXB(out, st0.FORMAT_CONTROL_2);
        SHOWXB(out, st0.FORMAT_CONTROL_3);
        SHOWCHARARRAY(out, st0.DEVICE_CLASS);
        SHOWXB(out, st0.NAMEPLATE_TYPE);
        SHOWXB(out, st0.DEFAULT_SET_USED);
        SHOWXB(out, st0.MAX_PROC_PARM_LENGTH);
        SHOWXB(out, st0.MAX_RESP_DATA_LEN);
        SHOWXB(out, st0.STD_VERSION_NO);
        SHOWXB(out, st0.STD_REVISION_NO);
        SHOWXB(out, st0.DIM_STD_TBLS_USED);
        SHOWXB(out, st0.DIM_MFG_TBLS_USED);
        SHOWXB(out, st0.DIM_STD_PROC_USED);
        SHOWXB(out, st0.DIM_MFG_PROC_USED);
        SHOWXB(out, st0.DIM_MFG_STATUS_USED);
        SHOWXB(out, st0.NBR_PENDING);
        SHOWBSET(out, st0.STD_TBLS_USED);
        SHOWBSET(out, st0.MFG_TBLS_USED);
        SHOWBSET(out, st0.STD_PROC_USED);
        SHOWBSET(out, st0.MFG_PROC_USED);
        SHOWBSET(out, st0.STD_TBLS_WRITE);
        SHOWBSET(out, st0.MFG_TBLS_WRITE);
        return out;
    }
};

struct ST_001_GENERAL_MFG_ID_TBL
{
    std::array<char,4> MANUFACTURER;    // This string is not zero-terminated
    std::array<char, 8>   ED_MODEL;        // This string is not zero-terminated
    uint8_t HW_VERSION_NUMBER;  // Hardware Version Number
    uint8_t HW_REVISION_NUMBER; // Hardware Revision Number
    uint8_t FW_VERSION_NUMBER;  // Firmware Version Number
    uint8_t FW_REVISION_NUMBER; // Firmware Revision Number
    std::array<char, 16>   MFG_SERIAL_NUMBER;

    ST_001_GENERAL_MFG_ID_TBL(std::string bytes) {
        if (bytes.size() >= sizeof(ST_001_GENERAL_MFG_ID_TBL)) {
            auto here{bytes.begin()};
            std::copy(here, here + MANUFACTURER.size(), MANUFACTURER.begin());
            here += MANUFACTURER.size();
            std::copy(here, here + ED_MODEL.size(), ED_MODEL.begin());
            here += ED_MODEL.size();
            HW_VERSION_NUMBER = *here++;
            HW_REVISION_NUMBER = *here++;
            FW_VERSION_NUMBER = *here++;
            FW_REVISION_NUMBER = *here++;
            std::copy(here, here + MFG_SERIAL_NUMBER.size(), MFG_SERIAL_NUMBER.begin());
            here += MFG_SERIAL_NUMBER.size();
        }
    }

    friend std::ostream& operator<<(std::ostream& out, const ST_001_GENERAL_MFG_ID_TBL& st1) {
        SHOWCHARARRAY(out, st1.MANUFACTURER);
        SHOWCHARARRAY(out, st1.ED_MODEL);
        SHOWXB(out, st1.HW_VERSION_NUMBER);
        SHOWXB(out, st1.HW_REVISION_NUMBER);
        SHOWXB(out, st1.FW_VERSION_NUMBER);
        SHOWXB(out, st1.FW_REVISION_NUMBER);
        SHOWCHARARRAY(out, st1.MFG_SERIAL_NUMBER);
        return out;
    }
};

struct ST_005_DEVICE_IDENT_TBL {
    std::array<char,20> IDENTIFICATION;    // This string is not zero-terminated

    ST_005_DEVICE_IDENT_TBL(std::string bytes) {
        if (bytes.size() >= sizeof(ST_005_DEVICE_IDENT_TBL)) {
            auto here{bytes.begin()};
            std::copy(here, here + IDENTIFICATION.size(), IDENTIFICATION.begin());
        }
    }

    friend std::ostream& operator<<(std::ostream& out, const ST_005_DEVICE_IDENT_TBL& st5) {
        SHOWCHARARRAY(out, st5.IDENTIFICATION);
        return out;
    }
};

class InterruptHandler // this is actually a singleton
{
   typedef void (*SignalHandlerType)(int);
   static SignalHandlerType s_previousInterruptHandler;
   static bool s_isInterrupted; // Here we know CEO is a singleton object.

   static void MyInterruptHandler(int)
   {
      s_isInterrupted = true;
   }

public:

   static bool IsInterrupted()
   {
      return s_isInterrupted;
   }

   static void ClearIsInterrupted()
   {
      s_isInterrupted = false;
   }

   InterruptHandler()
   {
      s_previousInterruptHandler = signal(SIGINT, MyInterruptHandler); // Handle Ctrl-C
      M_ASSERT(s_previousInterruptHandler != MyInterruptHandler); // check we did not call it twice
   }

   ~InterruptHandler()
   {
      signal(SIGINT, s_previousInterruptHandler); // restore signal
   }
};
bool InterruptHandler::s_isInterrupted = false;
InterruptHandler::SignalHandlerType InterruptHandler::s_previousInterruptHandler = nullptr;
static InterruptHandler s_interruptHandler;

unsigned failures = 0;
unsigned linkLayerRetries = 0;

static void CommitCommunication(MProtocol& proto)
{
   proto.QCommit(true);
   while ( !proto.QIsDone() )
   {
      linkLayerRetries = proto.GetCountLinkLayerPacketsRetried();
      MUtilities::Sleep(100);
      if ( s_interruptHandler.IsInterrupted() )
      {
         s_interruptHandler.ClearIsInterrupted();
         proto.GetChannel()->CancelCommunication(true);
      }
   }
   linkLayerRetries = proto.GetCountLinkLayerPacketsRetried();
}

void ReadItem(MProtocol& proto, MStdString item, unsigned count)
{
  try
  {
     if ( item.size() > 2 && !m_isdigit(item[0]) && !m_isdigit(item[0]) )
     {
        if ( item[1] == 'T' && (item[0] == 'S' || item[0] == 'M') ) // table read
        {
           int itemInt = MToLong(item.substr(2));
           if ( item[0] == 'M' )
              itemInt += 2048;
           proto.QTableRead(itemInt, 0, count);
        }
        else if ( item[1] == 'F' && (item[0] == 'S' || item[0] == 'M') )
        {
           MStdString::size_type openingBrace = item.find('(');
           MStdString::size_type closingBrace = item.find_last_of(')');
           if ( openingBrace == MStdString::npos || closingBrace == MStdString::npos || openingBrace >= closingBrace )
           {
              MException::Throw("Expected function syntax is like SF3(), MF150(01 02 03), ...");
           }
           int itemInt = MToLong(item.substr(2, openingBrace - 2));
           if ( item[0] == 'M' )
              itemInt += 2048;
           MByteString request;
           MStdString::size_type requestSize = closingBrace - openingBrace - 1;
           if ( requestSize > 0 )
              request = MUtilities::HexStringToBytes(item.substr(openingBrace + 1, requestSize));
           proto.QFunctionExecuteRequestResponse(itemInt, request, count);
        }
        else
           MException::Throw("Only prefixes supported are ST, MT, SF, MF");
     }
     else
     {
        int itemInt = MToLong(item);
        proto.QTableRead(itemInt, 0, count);
     }
  }
  catch ( MException& ex )
  {
     ex.Prepend("Bad syntax of argument '" + item + "': ");
     throw;
  }
}

static void Communicate(MProtocol& proto, const MStdStringVector& tables)
{
   proto.QConnect();
   proto.QStartSession();

   int count{1};
   for (const auto& item: tables) 
       ReadItem(proto, item, count++);

   proto.QEndSession();
   CommitCommunication(proto);
}

void GetResults(MProtocol& proto, const MStdStringVector& tables)
{
   auto it = tables.begin();
   for ( int count = 1; it != tables.cend(); ++it, ++count )
   {
      MStdString item = *it; // going to modify item
      int itemInt = -1;
      if ( item.size() > 2 && !m_isdigit(item[0]) && !m_isdigit(item[0]) )
      {
         if ( item[1] == 'T' && (item[0] == 'S' || item[0] == 'M') ) // table read
         {
            itemInt = MToLong(item.substr(2));
            if ( item[0] == 'M' )
               itemInt += 2048;
         }
      }
      else
         itemInt = MToLong(item);
      std::cout << item << ":\n"
           << MUtilities::BytesToHexString(proto.QGetTableData(itemInt, count),
                                           "  XX XX XX XX  XX XX XX XX  XX XX XX XX  XX XX XX XX\n")
           << std::endl;
      switch (itemInt) {
          case 0:
            std::cout << ST_000_GEN_CONFIG_TBL(proto.QGetTableData(itemInt, count));
            break;
          case 1:
            std::cout << ST_001_GENERAL_MFG_ID_TBL(proto.QGetTableData(itemInt, count));
            break;
          case 5:
            std::cout << ST_005_DEVICE_IDENT_TBL(proto.QGetTableData(itemInt, count));
            break;
          default:
            // do nothing
            break;
      }
   }

   // Report
   //
   MStdString str = "Device (unknown";
   str += ") errors/retries: ";
   str += MToStdString(failures);
   str += '/';
   str += MToStdString(linkLayerRetries);
   std::cout << str << std::endl;
   proto.WriteToMonitor(str);
}

int main(int argc, char** argv)
{
   Setup setup;
   if ( !setup.Initialize(argc, argv) )
      return EXIT_FAILURE;

   MProtocol* proto = setup.GetProtocol();
   M_ASSERT(proto != nullptr); // ensured by successful return from Initialize

   MProtocolC12* protoC12 = M_DYNAMIC_CAST(MProtocolC12, proto);
   if ( protoC12 != nullptr )
      protoC12->SetEndSessionOnApplicationLayerError(true); // this is the only property to override

   std::cout << "Entering test loop. Press Ctrl-C to interrupt." << std::endl;
  try
  {
     Communicate(*proto, setup.GetTableNames());
     GetResults(*proto, setup.GetTableNames());
  }
  catch ( MEOperationCancelled& )
  {
    std::cout << "Test loop is cancelled with Ctrl-C." << std::endl;
  }
  catch ( MException& ex )
  {
     std::cerr << "### Error: " << ex.AsString() << std::endl;
     ++failures;
  }

  proto->Disconnect(); // never throws

   std::cout << "Errors: " << failures
        << ", retries: " << proto->GetCountLinkLayerPacketsRetried()
        << '\n';
}
