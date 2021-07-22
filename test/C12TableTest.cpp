#include <iostream>
#include <iomanip>
#include <future>
#include <string>
#include <sstream>
#include "C12Tables.h"
#include <gtest/gtest.h>

using namespace C12;

class C12TableTest : public ::testing::Test {
protected:
    C12TableTest() : ST0{MakeST0(st0)}, MT0{MakeMT0(mt0)} {}
    const static std::basic_string<uint8_t> st0;
    const static std::basic_string<uint8_t> mt0;
    Table MakeST0(std::basic_string<uint8_t> tabledata);
    Table MakeMT0(std::basic_string<uint8_t> tabledata);
    Table ST0;
    Table MT0;
};

const std::basic_string<uint8_t> C12TableTest::st0 = 
{ 
     0x12,0x0A,0x9A,0x45, 0x50,0x52,0x49,0x02, 
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

Table C12TableTest::MakeST0(std::basic_string<uint8_t> tabledata) {
    Table ST0{0, "GEN_CONFIG_TBL", "GEN_CONFIG_RCD", tabledata}; 
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
    ST0.addField("STD_TBLS_USED", Table::fieldtype::SET, ST0.value("DIM_STD_TBLS_USED"));
    ST0.addField("MFG_TBLS_USED", Table::fieldtype::SET, ST0.value("DIM_MFG_TBLS_USED"));
    ST0.addField("STD_PROC_USED", Table::fieldtype::SET, ST0.value("DIM_STD_PROC_USED"));
    ST0.addField("MFG_PROC_USED", Table::fieldtype::SET, ST0.value("DIM_MFG_PROC_USED"));
    ST0.addField("STD_TBLS_WRITE", Table::fieldtype::SET, ST0.value("DIM_STD_TBLS_USED"));
    ST0.addField("MFG_TBLS_WRITE", Table::fieldtype::SET, ST0.value("DIM_MFG_TBLS_USED"));
    return ST0;
}

const std::basic_string<uint8_t> C12TableTest::mt0 = {
    0x07,0x00,0x01,0x02, 0x03,0x04,0x05,0x06, 
    0x48,0x65,0x6c,0x6c,0x6f,0x21,
    0x03,0x34,0x12,0xad, 0xde,0xfe,0xca
};

Table C12TableTest::MakeMT0(std::basic_string<uint8_t> tabledata) {
    Table MT0{0, "MY_TEST_TBL", "MY_TEST_RCD", tabledata}; 
    MT0.addField("DIM_ARRAY_ONE", Table::fieldtype::UINT, 1);
    MT0.addField("ARRAY_ONE", Table::fieldtype::UINT, 1, MT0.value("DIM_ARRAY_ONE"));
    MT0.addField("GREETING", Table::fieldtype::STRING, 6);
    MT0.addField("DIM_ARRAY_TWO", Table::fieldtype::UINT, 1);
    MT0.addField("ARRAY_TWO", Table::fieldtype::UINT, 2, MT0.value("DIM_ARRAY_TWO"));
    return MT0;
}

TEST_F(C12TableTest, tableName) {
    auto s = ST0.Name();
    EXPECT_EQ(s, "GEN_CONFIG_TBL");
}

TEST_F(C12TableTest, tableRecordName) {
    auto s = ST0.Record::Name();
    EXPECT_EQ(s, "GEN_CONFIG_RCD");
}

TEST_F(C12TableTest, fieldName) {
    auto s = ST0.front()->Name();
    EXPECT_EQ(s, "FORMAT_CONTROL_1");
}

TEST_F(C12TableTest, lastFieldName) {
    auto s = ST0.back()->Name();
    EXPECT_EQ(s, "MFG_TBLS_WRITE");
}

TEST_F(C12TableTest, tableValue) {
    auto s = ST0.value("DIM_STD_PROC_USED");
    EXPECT_EQ(s, 3);
}

TEST_F(C12TableTest, fieldToString) {
    auto s = ST0["DEVICE_CLASS"].value()->to_string(st0.data());
    EXPECT_EQ(s, "\"EPRI\"");
}

TEST_F(C12TableTest, fieldToStringSimple) {
    auto s = ST0.valueAsString("DEVICE_CLASS");
    EXPECT_EQ(s, "\"EPRI\"");
}

TEST_F(C12TableTest, strToString) {
    auto s = MT0.valueAsString("GREETING");
    EXPECT_EQ(s, "\"Hello!\"");
}

TEST_F(C12TableTest, strToChar) {
    auto s = MT0.value("GREETING", 1);
    EXPECT_EQ(s, 'e');
}

TEST_F(C12TableTest, extractBinaryValue) {
    EXPECT_EQ(ST0.value("DEVICE_CLASS", 0), 'E');
    EXPECT_EQ(ST0.value("DEVICE_CLASS", 1), 'P');
    EXPECT_EQ(ST0.value("DEVICE_CLASS", 2), 'R');
    EXPECT_EQ(ST0.value("DEVICE_CLASS", 3), 'I');
}

TEST_F(C12TableTest, fieldToValue) {
    auto s = ST0.value("NBR_PENDING");
    EXPECT_EQ(s, 6);
}

TEST_F(C12TableTest, tableFieldRef) {
    std::stringstream ss;
    ST0["DEVICE_CLASS"].value()->printTo(st0.data(), ss);
    std::string s{ss.str()};
    EXPECT_EQ(s, "\"EPRI\"");
}

TEST_F(C12TableTest, printBitfield) {
    std::stringstream ss;
    ST0["FORMAT_CONTROL_3"].value()->printTo(st0.data(), ss);
    std::string s{ss.str()};
    EXPECT_EQ(s, "{\n\tNI_FORMAT1 = 10\n\tNI_FORMAT2 = 9\n    }");
}

TEST_F(C12TableTest, bitfieldValue) {
    auto s = ST0.value("FORMAT_CONTROL_3", "NI_FORMAT2");
    EXPECT_EQ(s, 9);
}

TEST_F(C12TableTest, getBitfieldSize) {
    auto s = ST0["FORMAT_CONTROL_3"].value()->size();
    EXPECT_EQ(s, 1);
}

TEST_F(C12TableTest, getSetSizeNum) {
    auto s = ST0["STD_PROC_USED"].value()->size();
    EXPECT_EQ(s, 3);
}

TEST_F(C12TableTest, getSetSizeField) {
    auto s = ST0["STD_PROC_USED"].value()->size();
    EXPECT_EQ(s, ST0.value("DIM_STD_PROC_USED"));
}

TEST_F(C12TableTest, getSetValue) {
    EXPECT_EQ(ST0.value("STD_PROC_USED",0), false);
    EXPECT_EQ(ST0.value("STD_PROC_USED",1), false);
    EXPECT_EQ(ST0.value("STD_PROC_USED",2), false);
    EXPECT_EQ(ST0.value("STD_PROC_USED",3), true);
    EXPECT_EQ(ST0.value("STD_PROC_USED",4), true);
    EXPECT_EQ(ST0.value("STD_PROC_USED",5), true);
    EXPECT_EQ(ST0.value("STD_PROC_USED",6), true);
    EXPECT_EQ(ST0.value("STD_PROC_USED",7), true);
    EXPECT_EQ(ST0.value("STD_PROC_USED",8), true);
    EXPECT_EQ(ST0.value("STD_PROC_USED",9), true);
    EXPECT_EQ(ST0.value("STD_PROC_USED",10), true);
    EXPECT_EQ(ST0.value("STD_PROC_USED",11), true);
    EXPECT_EQ(ST0.value("STD_PROC_USED",12), true);
    EXPECT_EQ(ST0.value("STD_PROC_USED",13), false);
    EXPECT_EQ(ST0.value("STD_PROC_USED",14), true);
    EXPECT_EQ(ST0.value("STD_PROC_USED",15), false);
    EXPECT_EQ(ST0.value("STD_PROC_USED",16), false);
    EXPECT_EQ(ST0.value("STD_PROC_USED",17), false);
    EXPECT_EQ(ST0.value("STD_PROC_USED",18), false);
    EXPECT_EQ(ST0.value("STD_PROC_USED",19), false);
    EXPECT_EQ(ST0.value("STD_PROC_USED",20), true);
}

TEST_F(C12TableTest, printSet) {
    std::stringstream ss;
    ST0["STD_PROC_USED"].value()->printTo(st0.data(), ss);
    std::string s{ss.str()};
    EXPECT_EQ(s, "{ 3 4 5 6 7 8 9 10 11 12 14 20 }");
}

TEST_F(C12TableTest, fieldManfName) {
    auto s = MT0.front()->Name();
    EXPECT_EQ(s, "DIM_ARRAY_ONE");
}

TEST_F(C12TableTest, fieldManfNameSize1) {
    auto s = MT0.front()->size();
    EXPECT_EQ(s, 1);
}

TEST_F(C12TableTest, lastFieldManfName) {
    auto s = MT0.back()->Name();
    EXPECT_EQ(s, "ARRAY_TWO");
}

TEST_F(C12TableTest, getArrayValue) {
    auto s = MT0.value("ARRAY_ONE", 4);
    EXPECT_EQ(s, 4);
}

TEST_F(C12TableTest, fieldManfNameSize2) {
    auto s = MT0.back()->size();
    EXPECT_EQ(s, 6);
}

TEST_F(C12TableTest, getManfTableSize) {
    auto s = MT0.totalSize();
    EXPECT_EQ(s, mt0.size());
}

TEST_F(C12TableTest, getArrayValue2) {
    auto s = MT0.value("ARRAY_TWO", 2);
    EXPECT_EQ(s, 0xcafe);
}

TEST_F(C12TableTest, printTable) {
    std::stringstream ss;
    MT0.printTo(ss);
    std::string s{ss.str()};
    EXPECT_EQ(s, "TABLE 0 MY_TEST_TBL\n    DIM_ARRAY_ONE = 7\n    ARRAY_ONE = \n    ARRAY_ONE[0] = 7\n    ARRAY_ONE[1] = 0\n    ARRAY_ONE[2] = 1\n    ARRAY_ONE[3] = 2\n    ARRAY_ONE[4] = 3\n    ARRAY_ONE[5] = 4\n    ARRAY_ONE[6] = 5\n\n    GREETING = \"Hello!\"\n    DIM_ARRAY_TWO = 3\n    ARRAY_TWO = \n    ARRAY_TWO[0] = 7\n    ARRAY_TWO[1] = 513\n    ARRAY_TWO[2] = 1027\n\n");
}
