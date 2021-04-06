#ifndef C12TABLES_H
#define C12TABLES_H
#include <array>
#include <vector>
#include <iostream>
#include <initializer_list>

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

    ST_000_GEN_CONFIG_TBL(const std::string& bytes);
    ST_000_GEN_CONFIG_TBL(std::initializer_list<uint8_t> ll);

    friend std::ostream& operator<<(std::ostream& out, const ST_000_GEN_CONFIG_TBL& st0);
};

struct ST_001_GENERAL_MFG_ID_TBL
{
    std::array<char, 4> MANUFACTURER;    // This string is not zero-terminated
    std::array<char, 8>   ED_MODEL;      // This string is not zero-terminated
    uint8_t HW_VERSION_NUMBER;  // Hardware Version Number
    uint8_t HW_REVISION_NUMBER; // Hardware Revision Number
    uint8_t FW_VERSION_NUMBER;  // Firmware Version Number
    uint8_t FW_REVISION_NUMBER; // Firmware Revision Number
    std::array<char, 16>   MFG_SERIAL_NUMBER;

    ST_001_GENERAL_MFG_ID_TBL(std::string bytes);

    friend std::ostream& operator<<(std::ostream& out, const ST_001_GENERAL_MFG_ID_TBL& st1);
};

struct ST_005_DEVICE_IDENT_TBL {
    std::array<char,20> IDENTIFICATION;    // This string is not zero-terminated

    ST_005_DEVICE_IDENT_TBL(std::string bytes);

    friend std::ostream& operator<<(std::ostream& out, const ST_005_DEVICE_IDENT_TBL& st5);
};
#endif // C12TABLES_H
