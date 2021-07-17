#include "C12Tables.h"
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <iostream>
#include <sstream>

static bool global_big_endian{false};

namespace C12 {

    bool setDataOrder(bool big_endian) {
        return global_big_endian = big_endian;
    }

    static unsigned ReadUnsigned(const uint8_t* dataptr, std::size_t len, bool big_endian) {
        unsigned value{ 0 };
        if (big_endian) {
            for (std::size_t i{ 0 }; i < len; ++i) {
                value = (value << 8) | *dataptr++;
            }
        }
        else {
            dataptr += len - 1;
            for (std::size_t i{ 0 }; i < len; ++i) {
                value = (value << 8) | *dataptr--;
            }
        }
        return value;
    }

    std::string Field::to_string(const uint8_t* tabledata) const {
        std::stringstream ss;
        printTo(tabledata, ss);
        return ss.str();
    }

    UINT::UINT(std::string name, std::size_t offset, std::size_t len)
        : name{ name }
        , offset{ offset }
        , len{ len }
    {
    }

    unsigned UINT::operator()(const uint8_t* tabledata) const {
        return ReadUnsigned(tabledata + offset, len, global_big_endian);
    }

    std::ostream& UINT::printTo(const uint8_t* tabledata, std::ostream& out) const {
        return out << operator()(tabledata);
    }

    // we define a more efficient version of to_string() for UINT
    std::string UINT::to_string(const uint8_t* tabledata) const {
        return std::to_string(operator()(tabledata));
    }

    BINARY::BINARY(std::string name, std::size_t offset, std::size_t len)
        : name{ name }
        , offset{ offset }
        , len{ len }
    {
    }

    std::vector<uint8_t> BINARY::operator()(const uint8_t* tabledata) const {
        std::vector<uint8_t> v;
        v.reserve(len);
        std::copy(tabledata + offset, tabledata + offset + len, v.begin());
        return v;
    }

    std::ostream& BINARY::printTo(const uint8_t* tabledata, std::ostream& out) const {
        tabledata += offset;
        out << "\"";
        for (auto count{ len }; count; --count)
            out << static_cast<char>(*tabledata++);
        return out << "\"";
    }

    STRING::STRING(std::string name, std::size_t offset, std::size_t len)
        : name{ name }
        , offset{ offset }
        , len{ len }
    {
    }

    // we define a more efficient version of to_string() for STRING
    std::string STRING::to_string(const uint8_t* tabledata) const {
        return std::string{tabledata + offset, tabledata + offset + len};
    }

    std::vector<uint8_t> STRING::operator()(const uint8_t* tabledata) const {
        std::vector<uint8_t> v;
        v.reserve(len);
        std::copy(tabledata + offset, tabledata + offset + len, v.begin());
        return v;
    }

    std::ostream& STRING::printTo(const uint8_t* tabledata, std::ostream& out) const {
        tabledata += offset;
        out << "\"";
        for (auto count{ len }; count; --count)
            out << static_cast<char>(*tabledata++);
        return out << "\"";
    }

    SET::SET(std::string name, std::size_t offset, std::size_t len)
        : name{ name }
        , offset{ offset }
        , len{ len }
    {
    }

    std::vector<bool> SET::operator()(const uint8_t* tabledata) const {
        std::vector<bool> v;
        tabledata += offset;
        auto end = tabledata + len;
        v.reserve(len * 8);
        for (auto count{ len }; tabledata < end; ++tabledata) {
            for (uint8_t mask{ 1u }; mask; mask <<= 1) {
                v.push_back(*tabledata & mask);
            }
        }
        return v;
    }

    std::ostream& SET::printTo(const uint8_t* tabledata, std::ostream& out) const {
        auto bset{ operator()(tabledata) };
        out << "{ ";
        for (std::size_t i{ 0 }; i < bset.size(); ++i) {
            if (bset[i]) {
                out << i << ' ';
            }
        }
        return out << "}";
    }

    BITFIELD::BITFIELD(std::string name, std::size_t offset, std::size_t len)
        : name{ name }
        , offset{ offset }
        , len{ len }
    {
    }

    std::ostream& BITFIELD::printTo(const uint8_t* tabledata, std::ostream& out) const {
        out << "{\n";
        for (const auto& sub : subfields) {
            out << "\t" << sub.Name() << " = " << sub(ReadUnsigned(tabledata + offset, len, global_big_endian)) << '\n';
        }
        return out << "    }";
    }

    BITFIELD::Subfield::Subfield(std::string name, unsigned startbit, unsigned endbit)
        : name{ name }
        , shift{ startbit }
        , mask{ (1u << (endbit + 1 - startbit)) - 1 }
    {}

    unsigned BITFIELD::Subfield::operator()(unsigned fielddata) const {
        if (name == "DATA_ORDER") {
            global_big_endian = (fielddata >> shift) & mask;
        }
        return (fielddata >> shift) & mask;
    }

    void BITFIELD::addSubfield(std::string name, unsigned startbit, unsigned endbit) {
        subfields.emplace_back(Subfield{ name, startbit, endbit });
    }

    ARRAY::ARRAY(std::string name, std::size_t offset, Record::fieldtype type, std::size_t fieldsize, std::size_t count)
        : name{ name }
        , offset{ offset }
        , count{ count }
    {
        switch (type) {
        case Record::fieldtype::UINT:
            rec = std::make_unique<UINT>(name, 0, fieldsize);
            break;
        case Record::fieldtype::SET:
            rec = std::make_unique<SET>(name, 0, fieldsize);
            break;
        case Record::fieldtype::STRING:
            rec = std::make_unique<STRING>(name, 0, fieldsize);
            break;
        case Record::fieldtype::BINARY:
            rec = std::make_unique<BINARY>(name, 0, fieldsize);
            break;
        case Record::fieldtype::BITFIELD:
            rec = std::make_unique<BITFIELD>(name, 0, fieldsize);
            break;
        }
    }

    std::ostream& ARRAY::printTo(const uint8_t* tabledata, std::ostream& out) const {
        for (std::size_t i{0}; i < count; ++i) {
            out << "\n    " << Name() << "[" << i << "] = ";
            rec->printTo(tabledata + i * rec->size(), out);
        }
        return out << '\n';
    }

    std::unique_ptr<Field> ARRAY::clone() const {
        // TODO: recreate whatever rec is pointing to
        return std::unique_ptr<Field>(new ARRAY(name, offset, Record::fieldtype::STRING, 0, 0));
    }

    Record::Record(std::string name) 
        : name{ name }
    {
    }


    Table::Table(unsigned number, std::string name, std::string recordname, std::string data)
        : Record{ recordname }
        , num{ number }
        , name{ name }
        , data{ data.begin(), data.end() }
    {
    }

    Table::Table(unsigned number, std::string name, std::string recordname, std::basic_string<uint8_t> data)
        : Record{ recordname }
        , num{ number }
        , name{ name }
        , data{ data.begin(), data.end() }
    {
    }

    std::size_t Record::addField(std::string name, Record::fieldtype type, std::size_t fieldsize) {
        switch (type) {
        case Record::fieldtype::UINT:
            emplace_back(std::make_unique<UINT>(name, totalsize, fieldsize));
            break;
        case Record::fieldtype::SET:
            emplace_back(std::make_unique<SET>(name, totalsize, fieldsize));
            break;
        case Record::fieldtype::STRING:
            emplace_back(std::make_unique<STRING>(name, totalsize, fieldsize));
            break;
        case Record::fieldtype::BINARY:
            emplace_back(std::make_unique<BINARY>(name, totalsize, fieldsize));
            break;
        case Record::fieldtype::BITFIELD:
            emplace_back(std::make_unique<BITFIELD>(name, totalsize, fieldsize));
            break;
        }
        return totalsize += fieldsize;
    }
    std::size_t Record::addField(std::string name, Record::fieldtype type, std::size_t fieldsize, std::size_t arraysize) {
        emplace_back(std::make_unique<ARRAY>(name, totalsize, type, fieldsize, arraysize));
        return totalsize += fieldsize * arraysize;
    }

    std::ostream& Record::printTo(const std::string& str, std::ostream& out) const {
        return printTo(reinterpret_cast<const uint8_t*>(str.data()), out);
    }

    std::size_t Record::value(const uint8_t* tabledata, const std::string& fieldname) const {
        // find the field
        for (const auto& fld : *this) {
            if (fld->Name() == fieldname) {
                return fld->value(tabledata);
            }
        }
        return 0;
    }

    std::size_t Table::value(const std::string& fieldname) const {
        // find the field
        for (const auto& fld : *this) {
            if (fld->Name() == fieldname) {
                return fld->value(static_cast<const uint8_t *>(data.data()));
            }
        }
        return 0;
    }

    std::string Table::valueAsString(const std::string& fieldname) const {
        // find the field
        for (const auto& fld : *this) {
            if (fld->Name() == fieldname) {
                return fld->to_string(static_cast<const uint8_t *>(data.data()));
            }
        }
        return "";
    }

    std::ostream& Record::printTo(const uint8_t* tabledata, std::ostream& out) const {
        for (const auto& fld : *this) {
            out << "\n    " << fld->Name() << " = ";
            fld->printTo(tabledata, out);
        }
        return out << '\n';
    }

    std::ostream& Table::printTo(std::ostream& out) const {
        out << "TABLE " << num << ' ' << name;
        return Record::printTo(data.data(), out);
    }

    std::optional<std::unique_ptr<Field>> Record::operator[](const std::string& fieldname) const {
        for (const auto& fld : *this) {
            if (fld->Name() == fieldname) {
                return std::optional<std::unique_ptr<Field>>{fld->clone()};
            }
        }
        return std::nullopt;
    }

    void Record::addSubfield(const std::string& fieldname, std::string subfieldname, unsigned startbit, unsigned endbit) {
        for (const auto& fld : *this) {
            if (fld->Name() == fieldname) {
                fld->addSubfield(subfieldname, startbit, endbit);
                return;
            }
        }
    }

    void Record::addSubfield(const std::string& fieldname, std::string subfieldname, unsigned startbit) {
        addSubfield(fieldname, subfieldname, startbit, startbit);
    }
}
