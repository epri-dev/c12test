// File reader.cpp
//
// Universal configurable reader and link checker

#include <MCORE/MCOREExtern.h>
#include <MCOM/MCOM.h>
#include <fstream>
#include <signal.h>
#include <iostream>
#include "C12Tables.h"
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

Table MakeST0(const uint8_t *tabledata) {
    Table ST0{0, "GEN_CONFIG_TBL"}; 
    ST0.addField("FORMAT_CONTROL_1", Table::fieldtype::BITFIELD, 1);
    ST0.addSubfield("FORMAT_CONTROL_1", "DATA_ORDER", 0, 0);
    ST0.addSubfield("FORMAT_CONTROL_1", "CHAR_FORMAT", 1, 3);
    ST0.addSubfield("FORMAT_CONTROL_1", "MODEL_SELECT", 4, 6);
    ST0.addField("FORMAT_CONTROL_2", Table::fieldtype::BITFIELD, 1);
    ST0.addSubfield("FORMAT_CONTROL_2", "TM_FORMAT", 0, 2);
    ST0.addSubfield("FORMAT_CONTROL_2", "DATA_ACCESS_METHOD", 3, 4);
    ST0.addSubfield("FORMAT_CONTROL_2", "ID_FORM", 5, 5);
    ST0.addSubfield("FORMAT_CONTROL_2", "INT_FORMAT", 6, 7);
    ST0.addField("FORMAT_CONTROL_3", Table::fieldtype::BITFIELD, 1);
    ST0.addSubfield("FORMAT_CONTROL_3", "NI_FORMAT1", 0, 3);
    ST0.addSubfield("FORMAT_CONTROL_3", "NI_FORMAT2", 4, 7);
    ST0.addField("DEVICE_CLASS", Table::fieldtype::BINARY, 4);
    ST0.addField("NAMEPLATE_TYPE", Table::fieldtype::UINT, 1);
    ST0.addField("DEFAULT_SET_USED", Table::fieldtype::UINT, 1);
    ST0.addField("MAX_PROC_PARM_LENGTH", Table::fieldtype::UINT, 1);
    ST0.addField("MAX_RESP_DATA_LEN", Table::fieldtype::UINT, 1);
    ST0.addField("STD_VERSION_NO", Table::fieldtype::UINT, 1);
    ST0.addField("STD_REVISION_NO", Table::fieldtype::UINT, 1);
    ST0.addField("DIM_STD_TBLS_USED", Table::fieldtype::UINT, 1);
    ST0.addField("DIM_MFG_TBLS_USED", Table::fieldtype::UINT, 1);
    ST0.addField("DIM_STD_PROC_USED", Table::fieldtype::UINT, 1);
    ST0.addField("DIM_MFG_PROC_USED", Table::fieldtype::UINT, 1);
    ST0.addField("DIM_MFG_STATUS_USED", Table::fieldtype::UINT, 1);
    ST0.addField("NBR_PENDING", Table::fieldtype::UINT, 1);
    ST0.addField("STD_TBLS_USED", Table::fieldtype::SET, ST0.value(tabledata, "DIM_STD_TBLS_USED"));
    ST0.addField("MFG_TBLS_USED", Table::fieldtype::SET, ST0.value(tabledata, "DIM_MFG_TBLS_USED"));
    ST0.addField("STD_PROC_USED", Table::fieldtype::SET, ST0.value(tabledata, "DIM_STD_PROC_USED"));
    ST0.addField("MFG_PROC_USED", Table::fieldtype::SET, ST0.value(tabledata, "DIM_MFG_PROC_USED"));
    ST0.addField("STD_TBLS_WRITE", Table::fieldtype::SET, ST0.value(tabledata, "DIM_STD_TBLS_USED"));
    ST0.addField("MFG_TBLS_WRITE", Table::fieldtype::SET, ST0.value(tabledata, "DIM_MFG_TBLS_USED"));
    return ST0;
}

Table MakeST1() {
    Table ST1{1, "GENERAL_MFG_ID_TBL"}; 
    ST1.addField("MANUFACTURER", Table::fieldtype::STRING, 4);
    ST1.addField("ED_MODEL", Table::fieldtype::STRING, 8);
    ST1.addField("HW_VERSION_NUMBER", Table::fieldtype::UINT, 1);
    ST1.addField("HW_REVISION_NUMBER", Table::fieldtype::UINT, 1);
    ST1.addField("FW_VERSION_NUMBER", Table::fieldtype::UINT, 1);
    ST1.addField("FW_REVISION_NUMBER", Table::fieldtype::UINT, 1);
    ST1.addField("MFG_SERIAL_NUMBER", Table::fieldtype::STRING, 16);
    return ST1;
}

Table MakeST2() {
    Table ST2{2, "DEVICE_NAMEPLATE_TBL"}; 
    ST2.addField("E_KH", Table::fieldtype::STRING, 6);
    ST2.addField("E_KT", Table::fieldtype::STRING, 6);
    ST2.addField("E_INPUT_SCALAR", Table::fieldtype::UINT, 1);
    ST2.addField("E_ED_CONFIG", Table::fieldtype::STRING, 5);
    ST2.addField("E_ELEMENTS", Table::fieldtype::BITFIELD, 2);
    ST2.addSubfield("E_ELEMENTS", "E_FREQ", 0, 2);
    ST2.addSubfield("E_ELEMENTS", "E_NO_OF_ELEMENTS", 3, 5);
    ST2.addSubfield("E_ELEMENTS", "E_BASE_TYPE", 6, 9);
    ST2.addSubfield("E_ELEMENTS", "E_ACCURACY_CLASS", 10, 15);
    ST2.addField("E_VOLTS", Table::fieldtype::BITFIELD, 1);
    ST2.addSubfield("E_VOLTS", "E_ELEMENTS_VOLTS", 0, 3);
    ST2.addSubfield("E_VOLTS", "E_ED_SUPPLY_VOLTS", 4, 7);
    ST2.addField("E_CLASS_MAX_AMPS", Table::fieldtype::STRING, 6);
    ST2.addField("E_TA", Table::fieldtype::STRING, 6);
    return ST2;
}

Table MakeST5() {
    Table ST5{5, "DEVICE_IDENT_TBL"}; 
    ST5.addField("IDENTIFICATION", Table::fieldtype::STRING, 20);
    return ST5;
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
          case 5:
          { 
              auto ST5{MakeST5()};
              ST5.printTo(proto.QGetTableData(itemInt, count), std::cout);
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
