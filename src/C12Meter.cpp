#include "C12Meter.h"
#include <cctype>
#include <regex>
#include <signal.h>

unsigned linkLayerRetries = 0;

static C12::Table MakeST0(std::string tbldata, Meter& meter) {
    C12::Table ST0{0, "GEN_CONFIG_TBL", tbldata};
    ST0.addField("FORMAT_CONTROL_1", C12::Table::fieldtype::BITFIELD, 1);
    ST0.addSubfield("FORMAT_CONTROL_1", "DATA_ORDER", 0, 0);
    ST0.addSubfield("FORMAT_CONTROL_1", "CHAR_FORMAT", 1, 3);
    ST0.addSubfield("FORMAT_CONTROL_1", "MODEL_SELECT", 4, 6);
    ST0.addField("FORMAT_CONTROL_2", C12::Table::fieldtype::BITFIELD, 1);
    ST0.addSubfield("FORMAT_CONTROL_2", "TM_FORMAT", 0, 2);
    ST0.addSubfield("FORMAT_CONTROL_2", "DATA_ACCESS_METHOD", 3, 4);
    ST0.addSubfield("FORMAT_CONTROL_2", "ID_FORM", 5, 5);
    ST0.addSubfield("FORMAT_CONTROL_2", "INT_FORMAT", 6, 7);
    ST0.addField("FORMAT_CONTROL_3", C12::Table::fieldtype::BITFIELD, 1);
    ST0.addSubfield("FORMAT_CONTROL_3", "NI_FORMAT1", 0, 3);
    ST0.addSubfield("FORMAT_CONTROL_3", "NI_FORMAT2", 4, 7);
    ST0.addField("DEVICE_CLASS", C12::Table::fieldtype::BINARY, 4);
    ST0.addField("NAMEPLATE_TYPE", C12::Table::fieldtype::UINT, 1);
    ST0.addField("DEFAULT_SET_USED", C12::Table::fieldtype::UINT, 1);
    ST0.addField("MAX_PROC_PARM_LENGTH", C12::Table::fieldtype::UINT, 1);
    ST0.addField("MAX_RESP_DATA_LEN", C12::Table::fieldtype::UINT, 1);
    ST0.addField("STD_VERSION_NO", C12::Table::fieldtype::UINT, 1);
    ST0.addField("STD_REVISION_NO", C12::Table::fieldtype::UINT, 1);
    ST0.addField("DIM_STD_TBLS_USED", C12::Table::fieldtype::UINT, 1);
    ST0.addField("DIM_MFG_TBLS_USED", C12::Table::fieldtype::UINT, 1);
    ST0.addField("DIM_STD_PROC_USED", C12::Table::fieldtype::UINT, 1);
    ST0.addField("DIM_MFG_PROC_USED", C12::Table::fieldtype::UINT, 1);
    ST0.addField("DIM_MFG_STATUS_USED", C12::Table::fieldtype::UINT, 1);
    ST0.addField("NBR_PENDING", C12::Table::fieldtype::UINT, 1);
    ST0.addField("STD_TBLS_USED", C12::Table::fieldtype::SET, ST0.value("DIM_STD_TBLS_USED"));
    ST0.addField("MFG_TBLS_USED", C12::Table::fieldtype::SET, ST0.value("DIM_MFG_TBLS_USED"));
    ST0.addField("STD_PROC_USED", C12::Table::fieldtype::SET, ST0.value("DIM_STD_PROC_USED"));
    ST0.addField("MFG_PROC_USED", C12::Table::fieldtype::SET, ST0.value("DIM_MFG_PROC_USED"));
    ST0.addField("STD_TBLS_WRITE", C12::Table::fieldtype::SET, ST0.value("DIM_STD_TBLS_USED"));
    ST0.addField("MFG_TBLS_WRITE", C12::Table::fieldtype::SET, ST0.value("DIM_MFG_TBLS_USED"));
    return ST0;
}

static C12::Table MakeST1(std::string tbldata, Meter& meter) {
    C12::Table ST1{1, "GENERAL_MFG_ID_TBL", tbldata};
    ST1.addField("MANUFACTURER", C12::Table::fieldtype::STRING, 4);
    ST1.addField("ED_MODEL", C12::Table::fieldtype::STRING, 8);
    ST1.addField("HW_VERSION_NUMBER", C12::Table::fieldtype::UINT, 1);
    ST1.addField("HW_REVISION_NUMBER", C12::Table::fieldtype::UINT, 1);
    ST1.addField("FW_VERSION_NUMBER", C12::Table::fieldtype::UINT, 1);
    ST1.addField("FW_REVISION_NUMBER", C12::Table::fieldtype::UINT, 1);
    ST1.addField("MFG_SERIAL_NUMBER", C12::Table::fieldtype::STRING, 16);
    return ST1;
}

static C12::Table MakeST2(std::string tbldata, Meter& meter) {
    C12::Table ST2{2, "DEVICE_NAMEPLATE_TBL", tbldata};
    ST2.addField("E_KH", C12::Table::fieldtype::STRING, 6);
    ST2.addField("E_KT", C12::Table::fieldtype::STRING, 6);
    ST2.addField("E_INPUT_SCALAR", C12::Table::fieldtype::UINT, 1);
    ST2.addField("E_ED_CONFIG", C12::Table::fieldtype::STRING, 5);
    ST2.addField("E_ELEMENTS", C12::Table::fieldtype::BITFIELD, 2);
    ST2.addSubfield("E_ELEMENTS", "E_FREQ", 0, 2);
    ST2.addSubfield("E_ELEMENTS", "E_NO_OF_ELEMENTS", 3, 5);
    ST2.addSubfield("E_ELEMENTS", "E_BASE_TYPE", 6, 9);
    ST2.addSubfield("E_ELEMENTS", "E_ACCURACY_CLASS", 10, 15);
    ST2.addField("E_VOLTS", C12::Table::fieldtype::BITFIELD, 1);
    ST2.addSubfield("E_VOLTS", "E_ELEMENTS_VOLTS", 0, 3);
    ST2.addSubfield("E_VOLTS", "E_ED_SUPPLY_VOLTS", 4, 7);
    ST2.addField("E_CLASS_MAX_AMPS", C12::Table::fieldtype::STRING, 6);
    ST2.addField("E_TA", C12::Table::fieldtype::STRING, 6);
    return ST2;
}

static C12::Table MakeST3(std::string tbldata, Meter& meter) {
    C12::Table ST3{3, "ED_MODE_STATUS_TBL", tbldata};
    ST3.addField("ED_MODE", C12::Table::fieldtype::BITFIELD, 1);
    ST3.addSubfield("ED_MODE", "METERING_FLAG", 0);
    ST3.addSubfield("ED_MODE", "TEST_MODE_FLAG", 1);
    ST3.addSubfield("ED_MODE", "METER_SHOP_MODE_FLAG", 2);
    ST3.addSubfield("ED_MODE", "FACTORY_FLAG", 3);
    ST3.addField("ED_STD_STATUS1", C12::Table::fieldtype::BITFIELD, 2);
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
    ST3.addField("ED_STD_STATUS2", C12::Table::fieldtype::BITFIELD, 1);
    ST3.addField("ED_MFG_STATUS", C12::Table::fieldtype::BITFIELD, 1);
    return ST3;
}

static C12::Table MakeST5(std::string tbldata, Meter& meter) {
    C12::Table ST5{5, "DEVICE_IDENT_TBL", tbldata};
    ST5.addField("IDENTIFICATION", C12::Table::fieldtype::STRING, 20);
    return ST5;
}

static C12::Table MakeST6(std::string tbldata, Meter& meter) {
    C12::Table ST6{6, "UTIL_INFO_TBL", tbldata};
    ST6.addField("OWNER_NAME", C12::Table::fieldtype::STRING, 20);
    ST6.addField("UTILITY_DIV", C12::Table::fieldtype::STRING, 20);
    ST6.addField("SERVICE_POINT_ID", C12::Table::fieldtype::STRING, 20);
    ST6.addField("ELEC_ADDR", C12::Table::fieldtype::STRING, 20);
    ST6.addField("DEVICE_ID", C12::Table::fieldtype::STRING, 20);
    ST6.addField("UTIL_SER_NO", C12::Table::fieldtype::STRING, 20);
    ST6.addField("CUSTOMER_ID", C12::Table::fieldtype::STRING, 20);
    ST6.addField("COORDINATE_1", C12::Table::fieldtype::BINARY, 10);
    ST6.addField("COORDINATE_2", C12::Table::fieldtype::BINARY, 10);
    ST6.addField("COORDINATE_3", C12::Table::fieldtype::BINARY, 10);
    ST6.addField("TARIFF_ID", C12::Table::fieldtype::STRING, 8);
    ST6.addField("EX1_SW_VENDOR", C12::Table::fieldtype::STRING, 4);
    ST6.addField("EX1_SW_VERSION_NUMBER", C12::Table::fieldtype::UINT, 1);
    ST6.addField("EX1_SW_REVISION_NUMBER", C12::Table::fieldtype::UINT, 1);
    ST6.addField("EX2_SW_VENDOR", C12::Table::fieldtype::STRING, 4);
    ST6.addField("EX2_SW_VERSION_NUMBER", C12::Table::fieldtype::UINT, 1);
    ST6.addField("EX2_SW_REVISION_NUMBER", C12::Table::fieldtype::UINT, 1);
    ST6.addField("PROGRAMMER_NAME", C12::Table::fieldtype::STRING, 10);
    ST6.addField("MISC_ID", C12::Table::fieldtype::STRING, 30);
    return ST6;
}

static void AppendST20_tail(C12::Table& ST20) {
    ST20.addField("REG_FUNC1_FLAGS", C12::Table::fieldtype::BITFIELD, 1);
    ST20.addSubfield("REG_FUNC1_FLAGS", "SEASON_INFO_FIELD_FLAG", 0);
    ST20.addSubfield("REG_FUNC1_FLAGS", "DATA_TIME_FIELD_FLAG", 1);
    ST20.addSubfield("REG_FUNC1_FLAGS", "DEMAND_RESET_CTR_FLAG", 2);
    ST20.addSubfield("REG_FUNC1_FLAGS", "DEMAND_RESET_LOCK_FLAG", 3);
    ST20.addSubfield("REG_FUNC1_FLAGS", "CUM_DEMAND_FLAG", 4);
    ST20.addSubfield("REG_FUNC1_FLAGS", "CONT_CUM_DEMAND_FLAG", 5);
    ST20.addSubfield("REG_FUNC1_FLAGS", "TIME_REMAINING_FLAG", 6);
    ST20.addField("REG_FUNC2_FLAGS", C12::Table::fieldtype::BITFIELD, 1);
    ST20.addSubfield("REG_FUNC2_FLAGS", "SELF_READ_INHIBIT_OVERFLOW_FLAG", 0);
    ST20.addSubfield("REG_FUNC2_FLAGS", "SELF_READ_SEQ_NBR_FLAG", 1);
    ST20.addSubfield("REG_FUNC2_FLAGS", "DAILY_SELF_READ_FLAG", 2);
    ST20.addSubfield("REG_FUNC2_FLAGS", "WEEKLY_SELF_READ_FLAG", 3);
    ST20.addSubfield("REG_FUNC2_FLAGS", "SELF_READ_DEMAND_RESET", 4, 5);
    ST20.addField("NBR_SELF_READS", C12::Table::fieldtype::UINT, 1);
    ST20.addField("NBR_SUMMATIONS", C12::Table::fieldtype::UINT, 1);
    ST20.addField("NBR_DEMANDS", C12::Table::fieldtype::UINT, 1);
    ST20.addField("NBR_COIN_VALUES", C12::Table::fieldtype::UINT, 1);
    ST20.addField("NBR_OCCUR", C12::Table::fieldtype::UINT, 1);
    ST20.addField("NBR_TIERS", C12::Table::fieldtype::UINT, 1);
    ST20.addField("NBR_PRESENT_DEMANDS", C12::Table::fieldtype::UINT, 1);
    ST20.addField("NBR_PRESENT_VALUES", C12::Table::fieldtype::UINT, 1);
}

static C12::Table MakeST20(std::string tbldata, Meter& meter) {
    C12::Table ST20{20, "DIM_REGS_TBL", tbldata};
    AppendST20_tail(ST20);
    return ST20;
}

static C12::Table MakeST21(std::string tbldata, Meter& meter) {
    C12::Table ST21{21, "ACT_REGS_TBL", tbldata};
    AppendST20_tail(ST21);
    return ST21;
}

static void AppendST40_tail(C12::Table& ST40) {
    ST40.addField("NBR_PASSWORDS", C12::Table::fieldtype::UINT, 1);
    ST40.addField("PASSWORDS_LEN", C12::Table::fieldtype::UINT, 1);
    ST40.addField("NBR_KEYS", C12::Table::fieldtype::UINT, 1);
    ST40.addField("KEY_LEN", C12::Table::fieldtype::UINT, 1);
    ST40.addField("NBR_PERM_USED", C12::Table::fieldtype::UINT, 2);
}

static C12::Table MakeST40(std::string tbldata, Meter& meter) {
    C12::Table ST40{40, "DIM_SECURITY_LIMITING_TBL", tbldata};
    AppendST40_tail(ST40);
    return ST40;
}

static C12::Table MakeST41(std::string tbldata, Meter& meter) {
    C12::Table ST41{41, "ACT_SECURITY_LIMITING_TBL", tbldata};
    AppendST40_tail(ST41);
    return ST41;
}

static void AppendST50_tail(C12::Table& ST50) {
    ST50.addField("TIME_FUNC_FLAG1_BFLD", C12::Table::fieldtype::BITFIELD, 1);
    ST50.addSubfield("TIME_FUNC_FLAG1_BFLD", "TOU_SELF_READ_FLAG", 0);
    ST50.addSubfield("TIME_FUNC_FLAG1_BFLD", "SEASON_SELF_READ_FLAG", 1);
    ST50.addSubfield("TIME_FUNC_FLAG1_BFLD", "SEASON_DEMAND_RESET_FLAG", 2);
    ST50.addSubfield("TIME_FUNC_FLAG1_BFLD", "SEASON_CHNG_ARMED_FLAG", 3);
    ST50.addSubfield("TIME_FUNC_FLAG1_BFLD", "SORT_DATES_FLAG", 4);
    ST50.addSubfield("TIME_FUNC_FLAG1_BFLD", "ANCHOR_DATE_FLAG", 5);
    ST50.addField("TIME_FUNC_FLAG2_BFLD", C12::Table::fieldtype::BITFIELD, 1);
    ST50.addSubfield("TIME_FUNC_FLAG2_BFLD", "CAP_DST_AUTO_FLAG", 0);
    ST50.addSubfield("TIME_FUNC_FLAG2_BFLD", "SEPARATE_WEEKDAYS_FLAG", 1);
    ST50.addSubfield("TIME_FUNC_FLAG2_BFLD", "SEPARATE_SUM_DEMANDS_FLAG", 2);
    ST50.addSubfield("TIME_FUNC_FLAG2_BFLD", "SORT_TIER_SWITCHES_FLAG", 3);
    ST50.addSubfield("TIME_FUNC_FLAG2_BFLD", "CAP_TM_ZN_OFFSET_FLAG", 4);
    ST50.addField("CALENDAR_BFLD", C12::Table::fieldtype::BITFIELD, 1);
    ST50.addSubfield("CALENDAR_BFLD", "NBR_SEASONS", 0, 3);
    ST50.addSubfield("CALENDAR_BFLD", "NBR_SPECIAL_SCHED", 4, 7);
    ST50.addField("NBR_NON_RECURR_DATES", C12::Table::fieldtype::UINT, 1);
    ST50.addField("NBR_RECURR_DATES", C12::Table::fieldtype::UINT, 1);
    ST50.addField("NBR_TIER_SWITCHES", C12::Table::fieldtype::UINT, 2);
    ST50.addField("CALENDAR_TBL_SIZE", C12::Table::fieldtype::UINT, 2);
}

static C12::Table MakeST50(std::string tbldata, Meter& meter) {
    C12::Table ST50{50, "DIM_TIME_TOU_TBL", tbldata};
    AppendST50_tail(ST50);
    return ST50;
}

static C12::Table MakeST51(std::string tbldata, Meter& meter) {
    C12::Table ST51{51, "ACT_TIME_TOU_TBL", tbldata};
    AppendST50_tail(ST51);
    return ST51;
}

static C12::Table MakeST52(std::string tbldata, Meter& meter) {
    C12::Table ST52{52, "CLOCK_TBL", tbldata};
    // this is only valid when GEN_CONFIG_TBL.TM_FORMAT == 2
    //ST52.addField("CLOCK_CALENDAR", ltimedate);
    ST52.addField("CLOCK_CALENDAR.YEAR", C12::Table::fieldtype::UINT, 1);
    ST52.addField("CLOCK_CALENDAR.MONTH", C12::Table::fieldtype::UINT, 1);
    ST52.addField("CLOCK_CALENDAR.DAY", C12::Table::fieldtype::UINT, 1);
    ST52.addField("CLOCK_CALENDAR.HOUR", C12::Table::fieldtype::UINT, 1);
    ST52.addField("CLOCK_CALENDAR.MINUTE", C12::Table::fieldtype::UINT, 1);
    ST52.addField("CLOCK_CALENDAR.SECOND", C12::Table::fieldtype::UINT, 1);
    ST52.addField("TIME_DATE_QUAL", C12::Table::fieldtype::BITFIELD, 1);
    ST52.addSubfield("TIME_DATE_QUAL", "DAY_OF_WEEK", 0, 2);
    ST52.addSubfield("TIME_DATE_QUAL", "DST_FLAG", 3);
    ST52.addSubfield("TIME_DATE_QUAL", "GMT_FLAG", 4);
    ST52.addSubfield("TIME_DATE_QUAL", "TM_ZN_APPLIED_FLAG", 5);
    ST52.addSubfield("TIME_DATE_QUAL", "DST_APPLIED_FLAG", 6);
    return ST52;
}

static C12::Table MakeST55(std::string tbldata, Meter& meter) {
    C12::Table ST55{55, "CLOCK_STATE_TBL", tbldata};
    // this is only valid when GEN_CONFIG_TBL.TM_FORMAT == 2
    //ST55.addField("CLOCK_CALENDAR", ltimedate);
    ST55.addField("CLOCK_CALENDAR.YEAR", C12::Table::fieldtype::UINT, 1);
    ST55.addField("CLOCK_CALENDAR.MONTH", C12::Table::fieldtype::UINT, 1);
    ST55.addField("CLOCK_CALENDAR.DAY", C12::Table::fieldtype::UINT, 1);
    ST55.addField("CLOCK_CALENDAR.HOUR", C12::Table::fieldtype::UINT, 1);
    ST55.addField("CLOCK_CALENDAR.MINUTE", C12::Table::fieldtype::UINT, 1);
    ST55.addField("CLOCK_CALENDAR.SECOND", C12::Table::fieldtype::UINT, 1);
    ST55.addField("TIME_DATE_QUAL", C12::Table::fieldtype::BITFIELD, 1);
    ST55.addSubfield("TIME_DATE_QUAL", "DAY_OF_WEEK", 0, 2);
    ST55.addSubfield("TIME_DATE_QUAL", "DST_FLAG", 3);
    ST55.addSubfield("TIME_DATE_QUAL", "GMT_FLAG", 4);
    ST55.addSubfield("TIME_DATE_QUAL", "DST_APPLIED_FLAG", 6);
    ST55.addField("STATUS", C12::Table::fieldtype::BITFIELD, 2);
    // only valid when ACT_TIME_TOU_TBL>SEPARATE_SUM_DEMANDS_FLAG is false
    ST55.addSubfield("STATUS", "CURR_TIER", 0, 2);
    // ENDIF
    ST55.addSubfield("STATUS", "TIER_DRIVE", 6, 7);
    ST55.addSubfield("STATUS", "SPECIAL_SCHD_ACTIVE", 8, 11);
    ST55.addSubfield("STATUS", "SEASON", 12, 15);
    return ST55;
}

static C12::Table MakeST56(std::string tbldata, Meter& meter) {
    C12::Table ST56{56, "TIME_REMAIN_TBL", tbldata};
    // only valid when ACT_TIME_TOU_TBL>SEPARATE_SUM_DEMANDS_FLAG is false
    ST56.addField("TIER_TIME_REMAIN", C12::Table::fieldtype::UINT, 2);
    ST56.addField("SELF_READ_DAYS_REMAIN", C12::Table::fieldtype::UINT, 1);
    // ENDIF
    return ST56;
}

static void AppendST60_tail(C12::Table& ST60) {
    ST60.addField("LP_MEMORY_LEN", C12::Table::fieldtype::UINT, 4);
    ST60.addField("LP_FLAGS", C12::Table::fieldtype::BITFIELD, 2);
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
    ST60.addField("LP_FMATS", C12::Table::fieldtype::BITFIELD, 1);
    ST60.addSubfield("LP_FMATS", "INV_UINT8_FLAG", 0);
    ST60.addSubfield("LP_FMATS", "INV_UINT16_FLAG", 1);
    ST60.addSubfield("LP_FMATS", "INV_UINT32_FLAG", 2);
    ST60.addSubfield("LP_FMATS", "INV_INT8_FLAG", 3);
    ST60.addSubfield("LP_FMATS", "INV_INT16_FLAG", 4);
    ST60.addSubfield("LP_FMATS", "INV_INT32_FLAG", 5);
    ST60.addSubfield("LP_FMATS", "INV_NI_FMAT1_FLAG", 6);
    ST60.addSubfield("LP_FMATS", "INV_NI_FMAT2_FLAG", 7);
    // if ST64 is present
    ST60.addField("NBR_BLKS_SET1", C12::Table::fieldtype::UINT, 2);
    ST60.addField("NBR_BLK_INTS_SET1", C12::Table::fieldtype::UINT, 2);
    ST60.addField("NBR_CHNS_SET1", C12::Table::fieldtype::UINT, 1);
    ST60.addField("MAX_INT_TIME_SET1", C12::Table::fieldtype::UINT, 1);
    // if ST65 is present
#if ENABLEST65
    ST60.addField("NBR_BLKS_SET2", C12::Table::fieldtype::UINT, 2);
    ST60.addField("NBR_BLK_INTS_SET2", C12::Table::fieldtype::UINT, 2);
    ST60.addField("NBR_CHNS_SET2", C12::Table::fieldtype::UINT, 1);
    ST60.addField("MAX_INT_TIME_SET2", C12::Table::fieldtype::UINT, 1);
#endif
}

static C12::Table MakeST60(std::string tbldata, Meter& meter) {
    C12::Table ST60{60, "DIM_LP_TBL", tbldata};
    AppendST60_tail(ST60);
    return ST60;
}

static C12::Table MakeST61(std::string tbldata, Meter& meter) {
    C12::Table ST61{61, "ACT_LP_TBL", tbldata};
    AppendST60_tail(ST61);
    return ST61;
}

static void AppendST70_tail(C12::Table& ST70) {
    ST70.addField("LOG_FLAGS", C12::Table::fieldtype::BITFIELD, 2);
    ST70.addSubfield("LOG_FLAGS", "EVENT_NUMBER_FLAG", 0);
    ST70.addSubfield("LOG_FLAGS", "HIST_DATE_TIME_FLAG", 1);
    ST70.addSubfield("LOG_FLAGS", "HIST_SEQ_NBR_FLAG", 2);
    ST70.addSubfield("LOG_FLAGS", "HIST_INHIBIT_OVF_FLAG", 3);
    ST70.addSubfield("LOG_FLAGS", "EVENT_INHIBIT_OVF_FLAG", 4);
    ST70.addField("NBR_STD_EVENTS", C12::Table::fieldtype::UINT, 1);
    ST70.addField("NBR_MFG_EVENTS", C12::Table::fieldtype::UINT, 1);
    ST70.addField("HIST_DATA_LENGTH", C12::Table::fieldtype::UINT, 1);
    ST70.addField("EVENT_DATA_LENGTH", C12::Table::fieldtype::UINT, 1);
    ST70.addField("NBR_HISTORY_ENTRIES", C12::Table::fieldtype::UINT, 2);
    ST70.addField("NBR_EVENT_ENTRIES", C12::Table::fieldtype::UINT, 2);
}

static C12::Table MakeST70(std::string tbldata, Meter& meter) {
    C12::Table ST70{70, "DIM_LOG_TBL", tbldata};
    AppendST70_tail(ST70);
    return ST70;
}

static C12::Table MakeST71(std::string tbldata, Meter& meter) {
    C12::Table ST71{71, "ACT_LOG_TBL", tbldata};
    AppendST70_tail(ST71);
    return ST71;
}

static C12::Table MakeST72(std::string tbldata, Meter& meter) {
    C12::Table ST72{72, "EVENTS_ID_TBL", tbldata};
    ST72.addField("STD_EVENTS_SUPPORTED", C12::Table::fieldtype::SET, meter.evaluate("ACT_LOG_TBL.NBR_STD_EVENTS"));
    ST72.addField("MFG_EVENTS_SUPPORTED", C12::Table::fieldtype::SET, meter.evaluate("ACT_LOG_TBL.NBR_MFG_EVENTS"));
    return ST72;
}

static C12::Table MakeST73(std::string tbldata, Meter& meter) {
    C12::Table ST73{73, "EVENTS_ID_TBL", tbldata};
    ST73.addField("STD_EVENTS_MONITORED_FLAGS", C12::Table::fieldtype::SET, meter.evaluate("ACT_LOG_TBL.NBR_STD_EVENTS"));
    ST73.addField("MFG_EVENTS_MONITORED_FLAGS", C12::Table::fieldtype::SET, meter.evaluate("ACT_LOG_TBL.NBR_MFG_EVENTS"));
    ST73.addField("STD_TBLS_MONITORED_FLAGS", C12::Table::fieldtype::SET, meter.evaluate("GEN_CONFIG_TBL.DIM_STD_TBLS_USED"));
    ST73.addField("MFG_TBLS_MONITORED_FLAGS", C12::Table::fieldtype::SET, meter.evaluate("GEN_CONFIG_TBL.DIM_MFG_TBLS_USED"));
    ST73.addField("STD_PROC_MONITORED_FLAGS", C12::Table::fieldtype::SET, meter.evaluate("GEN_CONFIG_TBL.DIM_STD_PROC_USED"));
    ST73.addField("MFG_PROC_MONITORED_FLAGS", C12::Table::fieldtype::SET, meter.evaluate("GEN_CONFIG_TBL.DIM_MFG_PROC_USED"));
    return ST73;
}

static void ReadItem(MProtocol & proto, MStdString item, unsigned count)
{
    try {
        if (item.size() > 2 && !m_isdigit(item[0]) && !m_isdigit(item[0])) {
            if (item[1] == 'T' && (item[0] == 'S' || item[0] == 'M'))   // table read
            {
                int itemInt = MToLong(item.substr(2));
                if (item[0] == 'M')
                    itemInt += 2048;
                proto.QTableRead(itemInt, 0, count);
            } else if (item[1] == 'F' && (item[0] == 'S' || item[0] == 'M')) {
                MStdString::size_type openingBrace = item.find('(');
                MStdString::size_type closingBrace = item.find_last_of(')');
                if (openingBrace == MStdString::npos
                    || closingBrace == MStdString::npos
                    || openingBrace >= closingBrace) {
                    MException::
                        Throw
                        ("Expected function syntax is like SF3(), MF150(01 02 03), ...");
                }
                int itemInt = MToLong(item.substr(2, openingBrace - 2));
                if (item[0] == 'M')
                    itemInt += 2048;
                MByteString request;
                MStdString::size_type requestSize =
                    closingBrace - openingBrace - 1;
                if (requestSize > 0)
                    request =
                        MUtilities::HexStringToBytes(item.substr(openingBrace + 1, requestSize));
                proto.QFunctionExecuteRequestResponse(itemInt, request, count);
            } else
                MException::Throw("Only prefixes supported are ST, MT, SF, MF");
        } else {
            int itemInt = MToLong(item);
            proto.QTableRead(itemInt, 0, count);
        }
    }
    catch(MException & ex) {
        ex.Prepend("Bad syntax of argument '" + item + "': ");
        throw;
    }
}

class InterruptHandler          // this is actually a singleton
{
    typedef void (*SignalHandlerType)(int);
    static SignalHandlerType s_previousInterruptHandler;
    static bool s_isInterrupted;    // Here we know CEO is a singleton object.

    static void MyInterruptHandler(int) {
        s_isInterrupted = true;
     } 
 public:
    static bool IsInterrupted() {
        return s_isInterrupted;
    }

    static void ClearIsInterrupted() {
        s_isInterrupted = false;
    }

    InterruptHandler() {
        s_previousInterruptHandler = signal(SIGINT, MyInterruptHandler);    // Handle Ctrl-C
        M_ASSERT(s_previousInterruptHandler != MyInterruptHandler); // check we did not call it twice
    }

    ~InterruptHandler() {
        signal(SIGINT, s_previousInterruptHandler); // restore signal
    }
};

bool InterruptHandler::s_isInterrupted = false;
InterruptHandler::SignalHandlerType InterruptHandler::s_previousInterruptHandler = nullptr;
static InterruptHandler s_interruptHandler;

static void CommitCommunication(MProtocol& proto)
{
    proto.QCommit(true);
    while (!proto.QIsDone()) {
        linkLayerRetries = proto.GetCountLinkLayerPacketsRetried();
        MUtilities::Sleep(100);
        if (s_interruptHandler.IsInterrupted()) {
            s_interruptHandler.ClearIsInterrupted();
            proto.GetChannel()->CancelCommunication(true);
        }
    }
    linkLayerRetries = proto.GetCountLinkLayerPacketsRetried();
}

void Meter::Communicate(MProtocol& proto, const MStdStringVector& tables)
{
    proto.QConnect();
    proto.QStartSession();

    int count {1};
    for (const auto & item: tables)
        ReadItem(proto, item, count++);

    proto.QEndSession();
    CommitCommunication(proto);
}

/**
 * Convert from a string to table number.
 *
 * Strings must either be MTd+ or STd+ or d+
 * where 'MT' and 'ST' are those literal characters, 
 * and 'd+' represents one or more digits.
 *
 * @param tblid the passed string
 * @return table number or -1 on error
 */
static int stringToTableNumber(const MStdString& tblid) {
    enum States { Initial, Digits, NeedOneDigit, WaitForT, Error } state{States::Initial};
    long number = -1;
    long offset = 0;
    for (unsigned char ch : tblid) {
        switch (state) {
            case States::Initial: 
                if (ch == 'S') {  // standard table
                    state = States::WaitForT;
                } else if (ch == 'M') { // manufacturer table
                    offset = 2048;
                    state = States::WaitForT;
                } else if (std::isdigit(ch)) {
                    number = ch - '0';
                    state = States::Digits;
                } else {
                    state = States::Error;
                }
                break;
            case States::Digits:
                if (std::isdigit(ch)) {
                    number = 10 * number + (ch - '0');
                } else {
                    state = States::Error;
                }
                break;
            case States::NeedOneDigit:
                if (std::isdigit(ch)) {
                    number = ch - '0';
                    state = States::Digits;
                } else {
                    state = States::Error;
                }
                break;
            case States::WaitForT:
                if (ch == 'T') {
                    state = States::NeedOneDigit;
                } else {
                    state = States::Error;
                }
                break;
            case States::Error:
                return -1;
        }
    }
    return number + offset;
}

long Meter::evaluate(const std::string& expression) {
    std::regex field_regex("([A-Z_]+)\\.([A-Z_]+)");
    std::smatch pieces;
    std::string tblname, fieldname;
    if (std::regex_match(expression, pieces, field_regex)) {
        for (size_t i = 0; i < pieces.size(); ++i) {
#if VERBOSE_DEBUG            
            std::ssub_match sub_match = pieces[i];
            std::string piece = sub_match.str();
            std::cout << "  submatch " << i << ": " << piece << '\n';
#endif
            if (i == 1) {
                tblname = pieces[i].str();
            }
            if (i == 2) {
                fieldname = pieces[i].str();
            }
        }
    }   
    for (const auto& t : table) {
        if (t.Name() == tblname) {
            return t.value(fieldname);
        }
    }
    return 0;    
}

std::string Meter::evaluateAsString(const std::string& expression) const {
    std::regex field_regex("([A-Z_]+)\\.([A-Z_]+)");
    std::smatch pieces;
    std::string tblname, fieldname;
    if (std::regex_match(expression, pieces, field_regex)) {
        for (size_t i = 0; i < pieces.size(); ++i) {
#if VERBOSE_DEBUG            
            std::ssub_match sub_match = pieces[i];
            std::string piece = sub_match.str();
            std::cout << "  submatch " << i << ": " << piece << '\n';
#endif
            if (i == 1) {
                tblname = pieces[i].str();
            }
            if (i == 2) {
                fieldname = pieces[i].str();
            }
        }
    }   
    for (const auto& t : table) {
        if (t.Name() == tblname) {
            return t.valueAsString(fieldname);
        }
    }
    return "";    
}

void Meter::interpret(int itemInt, MProtocol& proto, int count) {
    switch (itemInt) {
    case 0:
        {
            auto ST0{MakeST0(proto.QGetTableData(itemInt, count), *this)};
            ST0.printTo(std::cout);
            table.push_back(std::move(ST0));
        }
        break;
    case 1:
        {
            auto ST1{MakeST1(proto.QGetTableData(itemInt, count), *this)};
            ST1.printTo(std::cout);
            table.push_back(std::move(ST1));
        }
        break;
    case 2:
        {
            auto ST2{MakeST2(proto.QGetTableData(itemInt, count), *this)};
            ST2.printTo(std::cout);
            table.push_back(std::move(ST2));
        }
        break;
    case 3:
        {
            auto ST3{MakeST3(proto.QGetTableData(itemInt, count), *this)};
            ST3.printTo(std::cout);
            table.push_back(std::move(ST3));
        }
        break;
    case 5:
        {
            auto ST5{MakeST5(proto.QGetTableData(itemInt, count), *this)};
            ST5.printTo(std::cout);
            table.push_back(std::move(ST5));
        }
        break;
    case 6:
        {
            auto ST6{MakeST6(proto.QGetTableData(itemInt, count), *this)};
            ST6.printTo(std::cout);
            table.push_back(std::move(ST6));
        }
        break;
    case 20:
        {
            auto ST20{MakeST20(proto.QGetTableData(itemInt, count), *this)};
            ST20.printTo(std::cout);
            table.push_back(std::move(ST20));
        }
        break;
    case 21:
        {
            auto ST21{MakeST21(proto.QGetTableData(itemInt, count), *this)};
            ST21.printTo(std::cout);
            table.push_back(std::move(ST21));
        }
        break;
    case 40:
        {
            auto ST40{MakeST40(proto.QGetTableData(itemInt, count), *this)};
            ST40.printTo(std::cout);
            table.push_back(std::move(ST40));
        }
        break;
    case 41:
        {
            auto ST41{MakeST41(proto.QGetTableData(itemInt, count), *this)};
            ST41.printTo(std::cout);
            table.push_back(std::move(ST41));
        }
        break;
    case 50:
        {
            auto ST50{MakeST50(proto.QGetTableData(itemInt, count), *this)};
            ST50.printTo(std::cout);
            table.push_back(std::move(ST50));
        }
        break;
    case 51:
        {
            auto ST51{MakeST51(proto.QGetTableData(itemInt, count), *this)};
            ST51.printTo(std::cout);
            table.push_back(std::move(ST51));
        }
        break;
    case 52:
        {
            auto ST52{MakeST52(proto.QGetTableData(itemInt, count), *this)};
            ST52.printTo(std::cout);
            table.push_back(std::move(ST52));
        }
        break;
    case 55:
        {
            auto ST55{MakeST55(proto.QGetTableData(itemInt, count), *this)};
            ST55.printTo(std::cout);
            table.push_back(std::move(ST55));
        }
        break;
    case 56:
        {
            auto ST56{MakeST56(proto.QGetTableData(itemInt, count), *this)};
            ST56.printTo(std::cout);
            table.push_back(std::move(ST56));
        }
        break;
    case 60:
        {
            auto ST60{MakeST60(proto.QGetTableData(itemInt, count), *this)};
            ST60.printTo(std::cout);
            table.push_back(std::move(ST60));
        }
        break;
    case 61:
        {
            auto ST61{MakeST61(proto.QGetTableData(itemInt, count), *this)};
            ST61.printTo(std::cout);
            table.push_back(std::move(ST61));
        }
        break;
    case 70:
        {
            auto ST70{MakeST70(proto.QGetTableData(itemInt, count), *this)};
            ST70.printTo(std::cout);
            table.push_back(std::move(ST70));
        }
        break;
    case 71:
        {
            auto ST71{MakeST71(proto.QGetTableData(itemInt, count), *this)};
            ST71.printTo(std::cout);
            table.push_back(std::move(ST71));
        }
        break;
    case 72:
        {
            auto ST72{MakeST72(proto.QGetTableData(itemInt, count), *this)};
            ST72.printTo(std::cout);
            table.push_back(std::move(ST72));
        }
        break;
    case 73:
        {
            auto ST73{MakeST73(proto.QGetTableData(itemInt, count), *this)};
            ST73.printTo(std::cout);
            table.push_back(std::move(ST73));
        }
        break;
    default:
        // do nothing
        break;
    }
}

void Meter::GetResults(MProtocol& proto, const MStdStringVector& tables)
{
    int count{0};
    for (const auto& item : tables) {
        ++count;
        auto itemInt{stringToTableNumber(item)};
        std::cout << item << ":\n"
            << MUtilities::BytesToHexString(proto.QGetTableData(itemInt, count),
                                            "  XX XX XX XX  XX XX XX XX  XX XX XX XX  XX XX XX XX\n")
            << '\n';
        interpret(itemInt, proto, count);
    }

    std::stringstream ss;
    ss << "Device (" << evaluateAsString("GENERAL_MFG_ID_TBL.ED_MODEL") << ") retries: " << linkLayerRetries << '\n';
    proto.WriteToMonitor(ss.str());
}
