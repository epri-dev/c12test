#include "C12Tables.h"
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <iostream>

#define SHOW(ostr, item) ostr << #item " = " << item << '\n'
#define SHOWXB(ostr, item) ostr << #item " = 0x" << std::hex << std::setfill('0') << std::setw(2) << (unsigned)item << '\n'
#define SHOWBSET(ostr, bset) ostr << #bset << std::dec << ", size = " << bset.size() << " = { "; \
    for (std::size_t i{0}; i < bset.size(); ++i) if (bset[i]) ostr << i << ' '; \
        ostr << "}\n";
#define SHOWCHARARRAY(ostr, carr) ostr << #carr << " = \""; \
    std::copy(carr.begin(), carr.end(), std::ostream_iterator<char>(ostr, "")); \
        ostr << "\"\n";

static std::vector<bool> fetchSet(std::string::const_iterator& begin, std::size_t size) {
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

static std::vector<bool> fetchSet(const uint8_t* begin, std::size_t size) {
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

ST_000_GEN_CONFIG_TBL::ST_000_GEN_CONFIG_TBL(const std::string& bytes) {
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

ST_000_GEN_CONFIG_TBL::ST_000_GEN_CONFIG_TBL(std::initializer_list<uint8_t> bytes) {
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


std::ostream& operator<<(std::ostream& out, const ST_000_GEN_CONFIG_TBL& st0) {
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


ST_001_GENERAL_MFG_ID_TBL::ST_001_GENERAL_MFG_ID_TBL(std::string bytes) {
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

std::ostream& operator<<(std::ostream& out, const ST_001_GENERAL_MFG_ID_TBL& st1) {
    SHOWCHARARRAY(out, st1.MANUFACTURER);
    SHOWCHARARRAY(out, st1.ED_MODEL);
    SHOWXB(out, st1.HW_VERSION_NUMBER);
    SHOWXB(out, st1.HW_REVISION_NUMBER);
    SHOWXB(out, st1.FW_VERSION_NUMBER);
    SHOWXB(out, st1.FW_REVISION_NUMBER);
    SHOWCHARARRAY(out, st1.MFG_SERIAL_NUMBER);
    return out;
}

ST_005_DEVICE_IDENT_TBL::ST_005_DEVICE_IDENT_TBL(std::string bytes) {
    if (bytes.size() >= sizeof(ST_005_DEVICE_IDENT_TBL)) {
        auto here{bytes.begin()};
        std::copy(here, here + IDENTIFICATION.size(), IDENTIFICATION.begin());
    }
}

std::ostream& operator<<(std::ostream& out, const ST_005_DEVICE_IDENT_TBL& st5) {
    SHOWCHARARRAY(out, st5.IDENTIFICATION);
    return out;
}

