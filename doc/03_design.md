# Software design notes {#design} #

## Protocol implementation ##
The protocol implementation uses the Honeywell/Elster [C12Adapter](https://github.com/beroset/C12Adapter) open source implementation, written in C++, Licensed under the MIT License, and modified slightly to fix some bugs and compiler warnings.

## Data interpretation ##
Data interpretation is done by reading the syntax from the standard itself.  The syntax in the standard is a modified version of the Pascal programming language, but no Pascal code exists in this software.  Rather, the standard was read an manually translated into code.  

Reading and writing of data is solely accomplished by the underlying C12Adapter.  The data interpreter operates as an overlay over the data read or written by C12Adapter and the meter reading and data interpretation are the two main functions of the `Meter` object.

As an example, the standard describes Standard Table 12 (ST12) as follows:

```
TYPE UOM_ENTRY_BFLD = BIT FIELD OF UINT32
    ID_CODE                 : UINT(0..7);
    TIME_BASE               : UINT(8..10);
    MULTIPLIER              : UINT(11..13);
    Q1_ACCOUNTABILITY       : BOOL(14);
    Q2_ACCOUNTABILITY       : BOOL(15);
    Q3_ACCOUNTABILITY       : BOOL(16);
    Q4_ACCOUNTABILITY       : BOOL(17);
    NET_FLOW_ACCOUNTABILITY : BOOL(18);
    SEGMENTATION            : UINT(19..21);
    HARMONIC                : BOOL(22);
    RESERVED                : FILL(28..30);
    NFS                     : BOOL(31);
    ID_RESOURCE             : UINT(23..27);
END;

TYPE UOM_ENTRY_RCD = PACKED RECORD
    UOM_ENTRY : ARRAY[ACT_SOURCES_LIM_TBL.NBR_UOM_ENTRIES] OF
                    UOM_ENTRY_BFLD;
END;

TABLE 12 UOM_ENTRY_TBL = UOM_ENTRY_RCD;
```

This is translated into the following code within the `C12Meter.cpp` file.

```
static C12::Table MakeST12(std::string tbldata, Meter& meter)
{
    C12::Table ST12{12, "UOM_ENTRY_TBL", "UOM_ENTRY_RCD", tbldata};
    ST12.addField("UOM_ENTRY", C12::Table::fieldtype::BITFIELD, 4, meter.evaluate("ACT_SOURCES_LIB_TBL.NBR_UOM_ENTRIES"));
    ST12.addSubfield("UOM_ENTRY", "ID_CODE", 0, 7);
    ST12.addSubfield("UOM_ENTRY", "TIME_BASE", 8, 10);
    ST12.addSubfield("UOM_ENTRY", "MULTIPLIER", 11, 13);
    ST12.addSubfield("UOM_ENTRY", "Q1_ACCOUNTABILITY", 14);
    ST12.addSubfield("UOM_ENTRY", "Q2_ACCOUNTABILITY", 15);
    ST12.addSubfield("UOM_ENTRY", "Q3_ACCOUNTABILITY", 16);
    ST12.addSubfield("UOM_ENTRY", "Q4_ACCOUNTABILITY", 17);
    ST12.addSubfield("UOM_ENTRY", "NET_FLOW_ACCOUNTABILITY", 18);
    ST12.addSubfield("UOM_ENTRY", "SEGMENTATION", 19, 21);
    ST12.addSubfield("UOM_ENTRY", "HARMONIC", 22);
    ST12.addSubfield("UOM_ENTRY", "ID_RESOURCE", 23, 27);
    ST12.addSubfield("UOM_ENTRY", "RESERVED", 28, 30);
    ST12.addSubfield("UOM_ENTRY", "NFS", 31);
    return ST12;
}
```

Note that the only difference in the code between declaring a single `C12::Table::fieldtype::BITFIELD` and a `C12::Table::fieldtype::ARRAY` of bitfields is the existence of the fourth argument to `C12::Table::addField`.

As mentioned in the description of [How to use this software](@ref using), this software is intended to be run on a Raspberry Pi with special hardware or on any Windows or Linux computer with a USB optical probe.  

## Further reading ##

[How to use the software](@ref using)
