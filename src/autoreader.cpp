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

using namespace C12;

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

void AppendST50_tail(Table& ST50) {
    ST50.addField("TIME_FUNC_FLAG1_BFLD", Table::fieldtype::BITFIELD, 1);
    ST50.addSubfield("TIME_FUNC_FLAG1_BFLD", "TOU_SELF_READ_FLAG", 0);
    ST50.addSubfield("TIME_FUNC_FLAG1_BFLD", "SEASON_SELF_READ_FLAG", 1);
    ST50.addSubfield("TIME_FUNC_FLAG1_BFLD", "SEASON_DEMAND_RESET_FLAG", 2);
    ST50.addSubfield("TIME_FUNC_FLAG1_BFLD", "SEASON_CHNG_ARMED_FLAG", 3);
    ST50.addSubfield("TIME_FUNC_FLAG1_BFLD", "SORT_DATES_FLAG", 4);
    ST50.addSubfield("TIME_FUNC_FLAG1_BFLD", "ANCHOR_DATE_FLAG", 5);
    ST50.addField("TIME_FUNC_FLAG2_BFLD", Table::fieldtype::BITFIELD, 1);
    ST50.addSubfield("TIME_FUNC_FLAG2_BFLD", "CAP_DST_AUTO_FLAG", 0);
    ST50.addSubfield("TIME_FUNC_FLAG2_BFLD", "SEPARATE_WEEKDAYS_FLAG", 1);
    ST50.addSubfield("TIME_FUNC_FLAG2_BFLD", "SEPARATE_SUM_DEMANDS_FLAG", 2);
    ST50.addSubfield("TIME_FUNC_FLAG2_BFLD", "SORT_TIER_SWITCHES_FLAG", 3);
    ST50.addSubfield("TIME_FUNC_FLAG2_BFLD", "CAP_TM_ZN_OFFSET_FLAG", 4);
    ST50.addField("CALENDAR_BFLD", Table::fieldtype::BITFIELD, 1);
    ST50.addSubfield("CALENDAR_BFLD", "NBR_SEASONS", 0, 3);
    ST50.addSubfield("CALENDAR_BFLD", "NBR_SPECIAL_SCHED", 4, 7);
    ST50.addField("NBR_NON_RECURR_DATES", Table::fieldtype::UINT, 1);
    ST50.addField("NBR_RECURR_DATES", Table::fieldtype::UINT, 1);
    ST50.addField("NBR_TIER_SWITCHES", Table::fieldtype::UINT, 2);
    ST50.addField("CALENDAR_TBL_SIZE", Table::fieldtype::UINT, 2);
}

Table MakeST50() {
    Table ST50{50, "DIM_TIME_TOU_TBL"};
    AppendST50_tail(ST50);
    return ST50;
}

Table MakeST51() {
    Table ST51{51, "ACT_TIME_TOU_TBL"};
    AppendST50_tail(ST51);
    return ST51;
}

Table MakeST52() {
    Table ST52{52, "CLOCK_TBL"};
    // this is only valid when GEN_CONFIG_TBL.TM_FORMAT == 2
    //ST52.addField("CLOCK_CALENDAR", ltimedate);
    ST52.addField("CLOCK_CALENDAR.YEAR", Table::fieldtype::UINT, 1);
    ST52.addField("CLOCK_CALENDAR.MONTH", Table::fieldtype::UINT, 1);
    ST52.addField("CLOCK_CALENDAR.DAY", Table::fieldtype::UINT, 1);
    ST52.addField("CLOCK_CALENDAR.HOUR", Table::fieldtype::UINT, 1);
    ST52.addField("CLOCK_CALENDAR.MINUTE", Table::fieldtype::UINT, 1);
    ST52.addField("CLOCK_CALENDAR.SECOND", Table::fieldtype::UINT, 1);
    ST52.addField("TIME_DATE_QUAL", Table::fieldtype::BITFIELD, 1);
    ST52.addSubfield("TIME_DATE_QUAL", "DAY_OF_WEEK", 0, 2);
    ST52.addSubfield("TIME_DATE_QUAL", "DST_FLAG", 3);
    ST52.addSubfield("TIME_DATE_QUAL", "GMT_FLAG", 4);
    ST52.addSubfield("TIME_DATE_QUAL", "TM_ZN_APPLIED_FLAG", 5);
    ST52.addSubfield("TIME_DATE_QUAL", "DST_APPLIED_FLAG", 6);
    return ST52;
}

Table MakeST55() {
    Table ST55{55, "CLOCK_STATE_TBL"};
    // this is only valid when GEN_CONFIG_TBL.TM_FORMAT == 2
    //ST55.addField("CLOCK_CALENDAR", ltimedate);
    ST55.addField("CLOCK_CALENDAR.YEAR", Table::fieldtype::UINT, 1);
    ST55.addField("CLOCK_CALENDAR.MONTH", Table::fieldtype::UINT, 1);
    ST55.addField("CLOCK_CALENDAR.DAY", Table::fieldtype::UINT, 1);
    ST55.addField("CLOCK_CALENDAR.HOUR", Table::fieldtype::UINT, 1);
    ST55.addField("CLOCK_CALENDAR.MINUTE", Table::fieldtype::UINT, 1);
    ST55.addField("CLOCK_CALENDAR.SECOND", Table::fieldtype::UINT, 1);
    ST55.addField("TIME_DATE_QUAL", Table::fieldtype::BITFIELD, 1);
    ST55.addSubfield("TIME_DATE_QUAL", "DAY_OF_WEEK", 0, 2);
    ST55.addSubfield("TIME_DATE_QUAL", "DST_FLAG", 3);
    ST55.addSubfield("TIME_DATE_QUAL", "GMT_FLAG", 4);
    ST55.addSubfield("TIME_DATE_QUAL", "DST_APPLIED_FLAG", 6);
    ST55.addField("STATUS", Table::fieldtype::BITFIELD, 2);
    // only valid when ACT_TIME_TOU_TBL>SEPARATE_SUM_DEMANDS_FLAG is false
    ST55.addSubfield("STATUS", "CURR_TIER", 0, 2);
    // ENDIF
    ST55.addSubfield("STATUS", "TIER_DRIVE", 6, 7);
    ST55.addSubfield("STATUS", "SPECIAL_SCHD_ACTIVE", 8, 11);
    ST55.addSubfield("STATUS", "SEASON", 12, 15);
    return ST55;
}

Table MakeST56() {
    Table ST56{56, "TIME_REMAIN_TBL"};
    // only valid when ACT_TIME_TOU_TBL>SEPARATE_SUM_DEMANDS_FLAG is false
    ST56.addField("TIER_TIME_REMAIN", Table::fieldtype::UINT, 2);
    ST56.addField("SELF_READ_DAYS_REMAIN", Table::fieldtype::UINT, 1);
    // ENDIF
    return ST56;
}

void AppendST60_tail(Table& ST60) {
    ST60.addField("LP_MEMORY_LEN", Table::fieldtype::UINT, 4);
    ST60.addField("LP_FLAGS", Table::fieldtype::BITFIELD, 2);
    ST60.addSubfield("LP_FLAGS", "LP_SET1_INHIBIT_OVF_FLAG", 0);
    ST60.addSubfield("LP_FLAGS", "LP_SET2_INHIBIT_OVF_FLAG", 1);
    ST60.addSubfield("LP_FLAGS", "LP_SET3_INHIBIT_OVF_FLAG", 2);
    ST60.addSubfield("LP_FLAGS", "LP_SET4_INHIBIT_OVF_FLAG", 3);
    ST60.addSubfield("LP_FLAGS", "BLK_END_READ_FLAG", 4);
    ST60.addSubfield("LP_FLAGS", "BLK_END_PULSE_FLAG", 5);
    ST60.addSubfield("LP_FLAGS", "SCALAR_DIVISOR_FLAG_SET1", 6);
    ST60.addSubfield("LP_FLAGS", "SCALAR_DIVISOR_FLAG_SET2", 7);
    ST60.addSubfield("LP_FLAGS", "SCALAR_DIVISOR_FLAG_SET3", 8);
    ST60.addSubfield("LP_FLAGS", "SCALAR_DIVISOR_FLAG_SET4", 9);
    ST60.addSubfield("LP_FLAGS", "EXTENDED_INT_STATUS_FLAG", 10);
    ST60.addSubfield("LP_FLAGS", "SIMPLE_INT_STATUS_FLAG", 11);
    ST60.addSubfield("LP_FLAGS", "BLK_END_RD_INDICATOR_FLAG", 12);
    ST60.addField("LP_FMATS", Table::fieldtype::BITFIELD, 1);
    ST60.addSubfield("LP_FMATS", "INV_UINT8_FLAG", 0);
    ST60.addSubfield("LP_FMATS", "INV_UINT16_FLAG", 1);
    ST60.addSubfield("LP_FMATS", "INV_UINT32_FLAG", 2);
    ST60.addSubfield("LP_FMATS", "INV_INT8_FLAG", 3);
    ST60.addSubfield("LP_FMATS", "INV_INT16_FLAG", 4);
    ST60.addSubfield("LP_FMATS", "INV_INT32_FLAG", 5);
    ST60.addSubfield("LP_FMATS", "INV_NI_FMAT1_FLAG", 6);
    ST60.addSubfield("LP_FMATS", "INV_NI_FMAT2_FLAG", 7);
    // if ST64 is present
    ST60.addField("NBR_BLKS_SET1", Table::fieldtype::UINT, 2);
    ST60.addField("NBR_BLK_INTS_SET1", Table::fieldtype::UINT, 2);
    ST60.addField("NBR_CHNS_SET1", Table::fieldtype::UINT, 1);
    ST60.addField("MAX_INT_TIME_SET1", Table::fieldtype::UINT, 1);
    // if ST65 is present
#if ENABLEST65
    ST60.addField("NBR_BLKS_SET2", Table::fieldtype::UINT, 2);
    ST60.addField("NBR_BLK_INTS_SET2", Table::fieldtype::UINT, 2);
    ST60.addField("NBR_CHNS_SET2", Table::fieldtype::UINT, 1);
    ST60.addField("MAX_INT_TIME_SET2", Table::fieldtype::UINT, 1);
#endif
}

Table MakeST60() {
    Table ST60{60, "DIM_LP_TBL"};
    AppendST60_tail(ST60);
    return ST60;
}

Table MakeST61() {
    Table ST61{61, "ACT_LP_TBL"};
    AppendST60_tail(ST61);
    return ST61;
}

void AppendST70_tail(Table& ST70) {
    ST70.addField("LOG_FLAGS", Table::fieldtype::BITFIELD, 2);
    ST70.addSubfield("LOG_FLAGS", "EVENT_NUMBER_FLAG", 0);
    ST70.addSubfield("LOG_FLAGS", "HIST_DATE_TIME_FLAG", 1);
    ST70.addSubfield("LOG_FLAGS", "HIST_SEQ_NBR_FLAG", 2);
    ST70.addSubfield("LOG_FLAGS", "HIST_INHIBIT_OVF_FLAG", 3);
    ST70.addSubfield("LOG_FLAGS", "EVENT_INHIBIT_OVF_FLAG", 4);
    ST70.addField("NBR_STD_EVENTS", Table::fieldtype::UINT, 1);
    ST70.addField("NBR_MFG_EVENTS", Table::fieldtype::UINT, 1);
    ST70.addField("HIST_DATA_LENGTH", Table::fieldtype::UINT, 1);
    ST70.addField("EVENT_DATA_LENGTH", Table::fieldtype::UINT, 1);
    ST70.addField("NBR_HISTORY_ENTRIES", Table::fieldtype::UINT, 2);
    ST70.addField("NBR_EVENT_ENTRIES", Table::fieldtype::UINT, 2);
}

Table MakeST70() {
    Table ST70{70, "DIM_LOG_TBL"};
    AppendST70_tail(ST70);
    return ST70;
}

Table MakeST71() {
    Table ST71{71, "ACT_LOG_TBL"};
    AppendST70_tail(ST71);
    return ST71;
}

Table MakeST72() {
    Table ST72{72, "EVENTS_ID_TBL"};
    // TODO: actual length is ACT_LOG_TBL.NBR_STD_EVENTS
    ST72.addField("STD_EVENTS_SUPPORTED", Table::fieldtype::SET, 1);
    // TODO: actual length is ACT_LOG_TBL.NBR_MFG_EVENTS
    ST72.addField("MFG_EVENTS_SUPPORTED", Table::fieldtype::SET, 2);
    return ST72;
}

Table MakeST73() {
    Table ST73{73, "EVENTS_ID_TBL"};
    // TODO: actual length is ACT_LOG_TBL.NBR_STD_EVENTS
    ST73.addField("STD_EVENTS_MONITORED_FLAGS", Table::fieldtype::SET, 1);
    // TODO: actual length is ACT_LOG_TBL.NBR_MFG_EVENTS
    ST73.addField("MFG_EVENTS_MONITORED_FLAGS", Table::fieldtype::SET, 2);
    // TODO: actual length is GEN_CONFIG_TBL.DIM_STD_TBLS_USED
    ST73.addField("STD_TBLS_MONITORED_FLAGS", Table::fieldtype::SET, 13);
    // TODO: actual length is GEN_CONFIG_TBL.DIM_MFG_TBLS_USED
    ST73.addField("MFG_TBLS_MONITORED_FLAGS", Table::fieldtype::SET, 13);
    // TODO: actual length is GEN_CONFIG_TBL.DIM_STD_PROC_USED
    ST73.addField("STD_PROC_MONITORED_FLAGS", Table::fieldtype::SET, 3);
    // TODO: actual length is GEN_CONFIG_TBL.DIM_MFG_PROC_USED
    ST73.addField("MFG_PROC_MONITORED_FLAGS", Table::fieldtype::SET, 5);
    return ST73;
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
