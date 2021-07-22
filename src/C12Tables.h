#ifndef C12TABLES_H
#define C12TABLES_H
#include <array>
#include <bitset>
#include <memory>
#include <vector>
#include <iostream>
#include <initializer_list>
#include <optional>
#include <string>

namespace C12 {

    bool setDataOrder(bool big_endian);

    struct Field {
        virtual std::string Name() const = 0;
        virtual std::ostream& printTo(const uint8_t* tabledata, std::ostream& out) const = 0;
        virtual unsigned value(const uint8_t*) const { return 0; }
        virtual std::size_t size() const = 0;
        virtual std::unique_ptr<Field> clone() const = 0;
        virtual void addSubfield(std::string, unsigned, unsigned) {}
        virtual std::string to_string(const uint8_t* tabledata) const;
    };

    class UINT : public Field {
    public:
        std::string Name() const override { return name; }
        UINT(std::string name, std::size_t offset, std::size_t len = 1);
        unsigned operator()(const uint8_t* tabledata) const;
        std::ostream& printTo(const uint8_t* tabledata, std::ostream& out) const override;
        unsigned value(const uint8_t* tbldata) const override { return operator()(tbldata); }
        std::size_t size() const override { return len; }
        std::unique_ptr<Field> clone() const override {
            return std::unique_ptr<Field>(new UINT{ *this });
        }
        std::string to_string(const uint8_t* tabledata) const override;
    private:
        std::string name;
        std::size_t offset;
        std::size_t len;
    };

    class BINARY : public Field {
    public:
        std::string Name() const override { return name; }
        BINARY(std::string name, std::size_t offset, std::size_t len = 1);
        std::vector<uint8_t> operator()(const uint8_t* tabledata) const;
        std::ostream& printTo(const uint8_t* tabledata, std::ostream& out) const override;
        std::size_t size() const override { return len; }
        std::unique_ptr<Field> clone() const override {
            return std::unique_ptr<Field>(new BINARY{ *this });
        }
    private:
        std::string name;
        std::size_t offset;
        std::size_t len;
    };

    class STRING : public Field {
    public:
        std::string Name() const override { return name; }
        STRING(std::string name, std::size_t offset, std::size_t len = 1);
        std::vector<uint8_t> operator()(const uint8_t* tabledata) const;
        std::ostream& printTo(const uint8_t* tabledata, std::ostream& out) const override;
        std::size_t size() const override { return len; }
        std::unique_ptr<Field> clone() const override {
            return std::unique_ptr<Field>(new STRING{ *this });
        }
        std::string to_string(const uint8_t* tabledata) const override;
    private:
        std::string name;
        std::size_t offset;
        std::size_t len;
    };

    class SET : public Field {
    public:
        std::string Name() const override { return name; }
        SET(std::string name, std::size_t offset, std::size_t len = 1);
        std::vector<bool> operator()(const uint8_t* tabledata) const;
        std::ostream& printTo(const uint8_t* tabledata, std::ostream& out) const override;
        std::size_t size() const override { return len; }
        std::unique_ptr<Field> clone() const override {
            return std::unique_ptr<Field>(new SET{ *this });
        }
    private:
        std::string name;
        std::size_t offset;
        std::size_t len;
    };

    /* a bitfield is a collection of named subfields */
    class BITFIELD : public Field {
    public:
        std::string Name() const override { return name; }
        BITFIELD(std::string name, std::size_t offset, std::size_t len = 1);
        std::ostream& printTo(const uint8_t* tabledata, std::ostream& out) const override;
        std::size_t size() const override { return len; }
        std::unique_ptr<Field> clone() const override {
            return std::unique_ptr<Field>(new BITFIELD{ *this });
        }
        // a subfield can be BOOL, UINT or FILL which is simply ignored
        class Subfield {
        public:
            Subfield(std::string name, unsigned startbit, unsigned endbit);
            std::string Name() const { return name; }
            unsigned operator()(unsigned fielddata) const;
        private:
            std::string name;
            unsigned shift;
            unsigned mask;
        };
        void addSubfield(std::string name, unsigned startbit, unsigned endbit) override;
    private:
        std::string name;
        std::size_t offset;
        std::size_t len;
        std::vector<Subfield> subfields;
    };

    class Record : public std::vector<std::unique_ptr<Field>> {
    public:
        enum class fieldtype { UINT, SET, BINARY, STRING, BITFIELD, ARRAY };
        Record(std::string name);
        std::size_t addField(std::string name, fieldtype type, std::size_t fieldsize);
        std::size_t addField(std::string name, fieldtype type, std::size_t fieldsize, std::size_t arraysize);
        std::ostream& printTo(const std::string& str, std::ostream& out) const;
        std::ostream& printTo(const uint8_t* tabledata, std::ostream& out) const;
        std::size_t value(const uint8_t* tabledata, const std::string& fieldname) const;
        std::optional<std::unique_ptr<Field>> operator[](const std::string& fieldname) const;
        void addSubfield(const std::string& fieldname, std::string subfieldname, unsigned startbit, unsigned endbit);
        void addSubfield(const std::string& fieldname, std::string subfieldname, unsigned startbit);
    private:
        std::string name;
        std::size_t totalsize = 0;
    };

    /* an ARRAY is a numbered list of one type of field */
    class ARRAY : public Field {
    public:
        std::string Name() const override { return name; }
        ARRAY(std::string name, std::size_t offset, Record::fieldtype type, std::size_t fieldsize, std::size_t count);
        std::ostream& printTo(const uint8_t* tabledata, std::ostream& out) const override;
        std::size_t size() const override { return rec->size() * count; }
        std::unique_ptr<Field> clone() const override;
    private:
        std::string name;
        std::size_t offset;
        std::size_t count;
        std::unique_ptr<Field> rec;
    };


    class Table : public Record {
    public:
        Table(unsigned number, std::string name, std::string recordname, std::string data);
        Table(unsigned number, std::string name, std::string recordname, std::basic_string<uint8_t> data);
        std::string Name() const { return name; }
        std::size_t value(const std::string& fieldname) const;
        std::string valueAsString(const std::string& fieldname) const;
        std::ostream& printTo(std::ostream& out) const;
        std::size_t totalSize() const;
    private:
        unsigned num = 0;
        std::string name{};
        std::size_t totalsize = 0;
        std::vector<uint8_t> data{};
    };
}

/*
 * What we would like to do is to read in a text file containing the 
 * document syntax form of a table and have it create the code that
 * would allow it to be neatly printed and formatted.
 *
 * Also, we would like to then apply this created definition to a
 * binary image of the table and have it apply the translation in
 * a human and machine readable format.
 *
 * The document syntax form of standard table 0 is shown below.
 */

/*
TYPE FORMAT_CONTROL_1_BFLD = BIT FIELD OF UINT8
    DATA_ORDER : UINT(0..0);
    CHAR_FORMAT : UINT(1..3);
    MODEL_SELECT : UINT(4..6);
    FILLER : FILL(7);
END;

TYPE FORMAT_CONTROL_2_BFLD = BIT FIELD OF UINT8
    TM_FORMAT : UINT(0..2);
    DATA_ACCESS_METHOD : UINT(3..4);
    ID_FORM : UINT(5..5);
    INT_FORMAT : UINT(6..7);
END;

TYPE FORMAT_CONTROL_3_BFLD = BIT FIELD OF UINT8
    NI_FORMAT1 : UINT(0..3);
    NI_FORMAT2 : UINT(4..7);
END;

TYPE GEN_CONFIG_RCD = PACKED RECORD
    FORMAT_CONTROL_1 : FORMAT_CONTROL_1_BFLD;
    FORMAT_CONTROL_2 : FORMAT_CONTROL_2_BFLD;
    FORMAT_CONTROL_3 : FORMAT_CONTROL_3_BFLD;
    DEVICE_CLASS : BINARY(4);
    NAMEPLATE_TYPE : UINT8;
    DEFAULT_SET_USED : UINT8;
    MAX_PROC_PARM_LENGTH : UINT8;
    MAX_RESP_DATA_LEN : UINT8;
    STD_VERSION_NO : UINT8;
    STD_REVISION_NO : UINT8;
    DIM_STD_TBLS_USED : UINT8;
    DIM_MFG_TBLS_USED : UINT8;
    DIM_STD_PROC_USED : UINT8;
    DIM_MFG_PROC_USED : UINT8;
    DIM_MFG_STATUS_USED : UINT8;
    NBR_PENDING : UINT8;
    STD_TBLS_USED : SET(GEN_CONFIG_TBL.DIM_STD_TBLS_USED);
    MFG_TBLS_USED : SET(GEN_CONFIG_TBL.DIM_MFG_TBLS_USED);
    STD_PROC_USED : SET(GEN_CONFIG_TBL.DIM_STD_PROC_USED);
    MFG_PROC_USED : SET(GEN_CONFIG_TBL.DIM_MFG_PROC_USED);
    STD_TBLS_WRITE : SET(GEN_CONFIG_TBL.DIM_STD_TBLS_USED);
    MFG_TBLS_WRITE : SET(GEN_CONFIG_TBL.DIM_MFG_TBLS_USED);
END;

TABLE 0 GEN_CONFIG_TBL = GEN_CONFIG_RCD;

    */

#endif // C12TABLES_H
