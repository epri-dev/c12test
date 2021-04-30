// File reader.cpp
//
// Universal configurable reader and link checker

#include <MCORE/MCOREExtern.h>
#include <MCOM/MCOM.h>
#include <fstream>
#include <signal.h>
#include <iostream>
#include "C12Tables.h"
#include "C12Meter.h"
#include "Setup.h"

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
          {
              std::string table{proto.QGetTableData(itemInt,count)};
              const uint8_t *ptr = reinterpret_cast<const uint8_t *>(table.data());
              auto ST0{MakeST0(ptr)};
              ST0.printTo(ptr, std::cout);
          }
            break;
          case 1:
          {
              auto ST1{MakeST1()};
              ST1.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
            break;
          case 2:
          {
              auto ST2{MakeST2()};
              ST2.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
            break;
          case 3:
          {
              auto ST3{MakeST3()};
              ST3.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
          break;
          case 5:
          {
              auto ST5{MakeST5()};
              ST5.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
            break;
          case 6:
          {
              auto ST6{MakeST6()};
              ST6.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
            break;
          case 20:
          {
              auto ST20{MakeST20()};
              ST20.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
            break;
          case 21:
          {
              auto ST21{MakeST21()};
              ST21.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
            break;
          case 40:
          {
              auto ST40{MakeST40()};
              ST40.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
            break;
          case 41:
          {
              auto ST41{MakeST41()};
              ST41.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
            break;
          case 50:
          {
              auto ST50{MakeST50()};
              ST50.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
            break;
          case 51:
          {
              auto ST51{MakeST51()};
              ST51.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
            break;
          case 52:
          {
              auto ST52{MakeST52()};
              ST52.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
            break;
          case 55:
          {
              auto ST55{MakeST55()};
              ST55.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
            break;
          case 56:
          {
              auto ST56{MakeST56()};
              ST56.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
            break;
          case 60:
          {
              auto ST60{MakeST60()};
              ST60.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
            break;
          case 61:
          {
              auto ST61{MakeST61()};
              ST61.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
            break;
          case 70:
          {
              auto ST70{MakeST70()};
              ST70.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
            break;
          case 71:
          {
              auto ST71{MakeST71()};
              ST71.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
            break;
          case 72:
          {
              auto ST72{MakeST72()};
              ST72.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
            break;
          case 73:
          {
              auto ST73{MakeST73()};
              ST73.printTo(proto.QGetTableData(itemInt, count), std::cout);
          }
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
