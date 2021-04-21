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

Table MakeST3() {
    Table ST3{3, "ED_MODE_STATUS_TBL"}; 
    ST3.addField("ED_MODE", Table::fieldtype::BITFIELD, 1);
    ST3.addSubfield("ED_MODE", "METERING_FLAG", 0);
    ST3.addSubfield("ED_MODE", "TEST_MODE_FLAG", 1);
    ST3.addSubfield("ED_MODE", "METER_SHOP_MODE_FLAG", 2);
    ST3.addSubfield("ED_MODE", "FACTORY_FLAG", 3);
    ST3.addField("ED_STD_STATUS1", Table::fieldtype::BITFIELD, 2);
    ST3.addSubfield("ED_STD_STATUS1", "UNPROGRAMMED_FLAG", 0);
    ST3.addSubfield("ED_STD_STATUS1", "CONFIGURATION_ERROR_FLAG", 1);
    ST3.addSubfield("ED_STD_STATUS1", "SELF_CHK_ERROR_FLAG", 2);
    ST3.addSubfield("ED_STD_STATUS1", "RAM_FAILURE_FLAG", 3);
    ST3.addSubfield("ED_STD_STATUS1", "ROM_FAILURE_FLAG", 4);
    ST3.addSubfield("ED_STD_STATUS1", "NONVOL_MEM_FAILURE_FLAG", 5);
    ST3.addSubfield("ED_STD_STATUS1", "CLOCK_ERROR_FLAG", 6);
    ST3.addSubfield("ED_STD_STATUS1", "MEASUREMENT_ERROR_FLAG", 7);
    ST3.addSubfield("ED_STD_STATUS1", "LOW_BATTERY_FLAG", 8);
    ST3.addSubfield("ED_STD_STATUS1", "LOW_LOSS_POTENTIAL_FLAG", 9);
    ST3.addSubfield("ED_STD_STATUS1", "DEMAND_OVERLOAD_FLAG", 10);
    ST3.addSubfield("ED_STD_STATUS1", "POWER_FAILURE_FLAG", 11);
    ST3.addSubfield("ED_STD_STATUS1", "TAMPER_DETECT_FLAG", 12);
    ST3.addSubfield("ED_STD_STATUS1", "REVERSE_ROTATION_FLAG", 13);
    ST3.addField("ED_STD_STATUS2", Table::fieldtype::BITFIELD, 1);
    ST3.addField("ED_MFG_STATUS", Table::fieldtype::BITFIELD, 1);
    return ST3;
}

Table MakeST5() {
    Table ST5{5, "DEVICE_IDENT_TBL"}; 
    ST5.addField("IDENTIFICATION", Table::fieldtype::STRING, 20);
    return ST5;
}

Table MakeST6() {
    Table ST6{6, "UTIL_INFO_TBL"}; 
    ST6.addField("OWNER_NAME", Table::fieldtype::STRING, 20);
    ST6.addField("UTILITY_DIV", Table::fieldtype::STRING, 20);
    ST6.addField("SERVICE_POINT_ID", Table::fieldtype::STRING, 20);
    ST6.addField("ELEC_ADDR", Table::fieldtype::STRING, 20);
    ST6.addField("DEVICE_ID", Table::fieldtype::STRING, 20);
    ST6.addField("UTIL_SER_NO", Table::fieldtype::STRING, 20);
    ST6.addField("CUSTOMER_ID", Table::fieldtype::STRING, 20);
    ST6.addField("COORDINATE_1", Table::fieldtype::BINARY, 10);
    ST6.addField("COORDINATE_2", Table::fieldtype::BINARY, 10);
    ST6.addField("COORDINATE_3", Table::fieldtype::BINARY, 10);
    ST6.addField("TARIFF_ID", Table::fieldtype::STRING, 8);
    ST6.addField("EX1_SW_VENDOR", Table::fieldtype::STRING, 4);
    ST6.addField("EX1_SW_VERSION_NUMBER", Table::fieldtype::UINT, 1);
    ST6.addField("EX1_SW_REVISION_NUMBER", Table::fieldtype::UINT, 1);
    ST6.addField("EX2_SW_VENDOR", Table::fieldtype::STRING, 4);
    ST6.addField("EX2_SW_VERSION_NUMBER", Table::fieldtype::UINT, 1);
    ST6.addField("EX2_SW_REVISION_NUMBER", Table::fieldtype::UINT, 1);
    ST6.addField("PROGRAMMER_NAME", Table::fieldtype::STRING, 10);
    ST6.addField("MISC_ID", Table::fieldtype::STRING, 30);
    return ST6;
}

void AppendST20_tail(Table& ST20) {
    ST20.addField("REG_FUNC1_FLAGS", Table::fieldtype::BITFIELD, 1);
    ST20.addSubfield("REG_FUNC1_FLAGS", "SEASON_INFO_FIELD_FLAG", 0);
    ST20.addSubfield("REG_FUNC1_FLAGS", "DATA_TIME_FIELD_FLAG", 1);
    ST20.addSubfield("REG_FUNC1_FLAGS", "DEMAND_RESET_CTR_FLAG", 2);
    ST20.addSubfield("REG_FUNC1_FLAGS", "DEMAND_RESET_LOCK_FLAG", 3);
    ST20.addSubfield("REG_FUNC1_FLAGS", "CUM_DEMAND_FLAG", 4);
    ST20.addSubfield("REG_FUNC1_FLAGS", "CONT_CUM_DEMAND_FLAG", 5);
    ST20.addSubfield("REG_FUNC1_FLAGS", "TIME_REMAINING_FLAG", 6);
    ST20.addField("REG_FUNC2_FLAGS", Table::fieldtype::BITFIELD, 1);
    ST20.addSubfield("REG_FUNC2_FLAGS", "SELF_READ_INHIBIT_OVERFLOW_FLAG", 0);
    ST20.addSubfield("REG_FUNC2_FLAGS", "SELF_READ_SEQ_NBR_FLAG", 1);
    ST20.addSubfield("REG_FUNC2_FLAGS", "DAILY_SELF_READ_FLAG", 2);
    ST20.addSubfield("REG_FUNC2_FLAGS", "WEEKLY_SELF_READ_FLAG", 3);
    ST20.addSubfield("REG_FUNC2_FLAGS", "SELF_READ_DEMAND_RESET", 4, 5);
    ST20.addField("NBR_SELF_READS", Table::fieldtype::UINT, 1);
    ST20.addField("NBR_SUMMATIONS", Table::fieldtype::UINT, 1);
    ST20.addField("NBR_DEMANDS", Table::fieldtype::UINT, 1);
    ST20.addField("NBR_COIN_VALUES", Table::fieldtype::UINT, 1);
    ST20.addField("NBR_OCCUR", Table::fieldtype::UINT, 1);
    ST20.addField("NBR_TIERS", Table::fieldtype::UINT, 1);
    ST20.addField("NBR_PRESENT_DEMANDS", Table::fieldtype::UINT, 1);
    ST20.addField("NBR_PRESENT_VALUES", Table::fieldtype::UINT, 1);
}

Table MakeST20() {
    Table ST20{20, "DIM_REGS_TBL"}; 
    AppendST20_tail(ST20);
    return ST20;
}

Table MakeST21() {
    Table ST21{21, "ACT_REGS_TBL"}; 
    AppendST20_tail(ST21);
    return ST21;
}

void AppendST40_tail(Table& ST40) {
    ST40.addField("NBR_PASSWORDS", Table::fieldtype::UINT, 1);
    ST40.addField("PASSWORDS_LEN", Table::fieldtype::UINT, 1);
    ST40.addField("NBR_KEYS", Table::fieldtype::UINT, 1);
    ST40.addField("KEY_LEN", Table::fieldtype::UINT, 1);
    ST40.addField("NBR_PERM_USED", Table::fieldtype::UINT, 2);
}

Table MakeST40() {
    Table ST40{40, "DIM_SECURITY_LIMITING_TBL"}; 
    AppendST40_tail(ST40);
    return ST40;
}

Table MakeST41() {
    Table ST41{41, "ACT_SECURITY_LIMITING_TBL"}; 
    AppendST40_tail(ST41);
    return ST41;
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
