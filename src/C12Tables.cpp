#include "C12Tables.h"
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <iostream>

UINT::UINT(std::string name, std::size_t offset, std::size_t len, bool little_endian)
    : name{name}
    , offset{offset}
    , len{len}
    , little_endian{little_endian}
{
}

unsigned UINT::operator()(const uint8_t *tabledata) const {
    unsigned value{0};
    tabledata += offset;
    if (little_endian) {
        for (std::size_t i{0}; i < len; ++i) {
            value = (value << 8) | *tabledata++;
        }
    } else {
        tabledata += len - 1;
        for (std::size_t i{0}; i < len; ++i) {
            value = (value << 8) | *tabledata--;
        }
    }
    return value;
}

std::ostream& UINT::printTo(const uint8_t *tabledata, std::ostream& out) const {
    return out << operator()(tabledata);
}

BINARY::BINARY(std::string name, std::size_t offset, std::size_t len)
    : name{name}
    , offset{offset}
    , len{len}
{
}

std::vector<uint8_t> BINARY::operator()(const uint8_t *tabledata) const {
    std::vector<uint8_t> v;
    v.reserve(len);
    std::copy(tabledata + offset, tabledata + offset + len, v.begin());
    return v;
}

std::ostream& BINARY::printTo(const uint8_t *tabledata, std::ostream& out) const {
    tabledata += offset;
    out << "\"";
    for (auto count{len}; count; --count)
        out << static_cast<char>(*tabledata++);
    return out << "\"";
}

STRING::STRING(std::string name, std::size_t offset, std::size_t len)
    : name{name}
    , offset{offset}
    , len{len}
{
}

std::vector<uint8_t> STRING::operator()(const uint8_t *tabledata) const {
    std::vector<uint8_t> v;
    v.reserve(len);
    std::copy(tabledata + offset, tabledata + offset + len, v.begin());
    return v;
}

std::ostream& STRING::printTo(const uint8_t *tabledata, std::ostream& out) const {
    tabledata += offset;
    out << "\"";
    for (auto count{len}; count; --count)
        out << static_cast<char>(*tabledata++);
    return out << "\"";
}

SET::SET(std::string name, std::size_t offset, std::size_t len)
    : name{name}
    , offset{offset}
    , len{len}
{
}

std::vector<bool> SET::operator()(const uint8_t *tabledata) const {
    std::vector<bool> v;
    auto end = tabledata + len;
    v.reserve(len * 8);
    for (auto count{len}; tabledata < end; ++tabledata) {
        for (uint8_t mask{1}; mask; mask <<= 1) {
            if (count--)
                v.push_back(*tabledata & mask);
            else
                break;
        }
    }
    return v;
}

std::ostream& SET::printTo(const uint8_t *tabledata, std::ostream& out) const {
    auto bset{operator()(tabledata)};
    out << "{ "; 
    for (std::size_t i{0}; i < bset.size(); ++i) {
        if (bset[i]) {
            out << i << ' '; 
        }
    }
    return out << "}";
}

std::size_t Table::addField(std::string name, Table::fieldtype type, std::size_t fieldsize) {
    switch (type) {
        case Table::fieldtype::UINT:
            emplace_back(std::make_unique<UINT>(name, totalsize, fieldsize));
            break;
        case Table::fieldtype::SET:
            emplace_back(std::make_unique<SET>(name, totalsize, fieldsize));
            break;
        case Table::fieldtype::STRING:
            emplace_back(std::make_unique<STRING>(name, totalsize, fieldsize));
            break;
        case Table::fieldtype::BINARY:
            emplace_back(std::make_unique<BINARY>(name, totalsize, fieldsize));
            break;
    }
    return totalsize += fieldsize;
}

std::ostream& Table::printTo(const std::string& str, std::ostream& out) const {
    return printTo(reinterpret_cast<const uint8_t *>(str.data()), out);
}

std::ostream& Table::printFieldTo(const uint8_t *tabledata, std::ostream& out, std::string fieldname) const {
    for (const auto& fld: *this) {
        if (fld->Name() == fieldname) {
            fld->printTo(tabledata, out);
        }
    }
    return out;
}

std::size_t Table::value(const uint8_t *tabledata, const std::string& fieldname) const {
    // find the field
    for (const auto& fld: *this) {
        if (fld->Name() == fieldname) {
            return fld->value(tabledata);
        }
    }
    return 0;
}

std::ostream& Table::printTo(const uint8_t *tabledata, std::ostream& out) const {
    out << "TABLE " << num << ' ' << name;
    for (const auto& fld : *this) {
        out << "\n    " << fld->Name() << " = ";
        fld->printTo(tabledata, out);
    }
    return out << '\n';
}
