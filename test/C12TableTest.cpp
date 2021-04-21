#include <iostream>
#include <iomanip>
#include <future>
#include <string>
#include <sstream>
#include "C12Tables.h"
#include <gtest/gtest.h>

static std::basic_string<uint8_t> st0{ 
     0x12,0x0A,0x9A,0x45, 0x45,0x20,0x20,0x02, 
     0x00,0x13,0x18,0x01, 0x00,0x0D,0x0D,0x03,
     0x05,0x0D,0x06,0xFF, 0xAD,0xF0,0xDF,0x03, 
     0x3F,0xFC,0xF0,0xC1, 0x1F,0xFF,0xFF,0x03,
     0x3E,0xFF,0xAF,0xA2, 0x01,0x85,0xFF,0xFF, 
     0x1F,0x30,0x8F,0xFF, 0xF7,0xF8,0x5F,0x10,
     0xFE,0xFF,0x1E,0x16, 0xDB,0xE0,0xA8,0xE0, 
     0x08,0x03,0x34,0x68, 0x60,0x80,0x0A,0xFC,
     0xF3,0x00,0x24,0xA5, 0x00,0xA0,0x01,0x81, 
     0x19,0x67,0x10,0x00, 0x82,0xF5,0xE0,
};

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

TEST(C12TableTest, testTableFieldRef) {
    auto ST0 = MakeST0(st0.data());
    std::stringstream ss;
    ST0["DEVICE_CLASS"].value()->printTo(st0.data(), ss);
    std::string s{ss.str()};
    EXPECT_EQ(s, "\"EE  \"");
}

