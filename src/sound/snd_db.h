#pragma once

struct snd_group;
struct snd_curve;
struct snd_pan;
struct snd_snapshot_group;
struct snd_alias_t;


enum snd_csv_entry_type : __int32
{                                       // XREF: snd_csv_entry_t/r
    SND_CSV_STRING = 0x0,            // XREF: .rdata:snd_csv_entry_t const * const SND_ALIAS_STRINGS_FIELDS/s
    // .rdata:snd_csv_entry_t const * const SND_GROUP_FIELDS/s ...
    SND_CSV_FLOAT = 0x1,            // XREF: .rdata:snd_csv_entry_t const * const SND_CURVE_FIELDS/s
    // .rdata:snd_csv_entry_t const * const SND_PAN_FIELDS/s ...
    SND_CSV_INT = 0x2,
    SND_CSV_ENUM = 0x3,            // XREF: .rdata:snd_csv_entry_t const * const SND_ALIAS_STRINGS_FIELDS/s
    // .rdata:snd_csv_entry_t const * const SND_GROUP_FIELDS/s
    SND_CSV_FLAG = 0x4,
    SND_CSV_DBSPL = 0x5,            // XREF: .rdata:snd_csv_entry_t const * const SND_ALIAS_FIELDS/s
    // .rdata:snd_csv_entry_t const * const SND_GROUP_FIELDS/s
    SND_CSV_CENTS = 0x6,            // XREF: .rdata:snd_csv_entry_t const * const SND_ALIAS_FIELDS/s
    SND_CSV_HASH = 0x7,            // XREF: .rdata:snd_csv_entry_t const * const SND_ALIAS_FIELDS/s
    // .rdata:snd_csv_entry_t const * const SND_GROUP_FIELDS/s ...
    SND_CSV_ENUM_BITS = 0x8,            // XREF: .rdata:snd_csv_entry_t const * const SND_ALIAS_FIELDS/s
    SND_CSV_FLAG_BITS = 0x9,            // XREF: .rdata:snd_csv_entry_t const * const SND_ALIAS_FIELDS/s
    SND_CSV_BYTE = 0xA,            // XREF: .rdata:snd_csv_entry_t const * const SND_ALIAS_FIELDS/s
    SND_CSV_ENUM_BYTE = 0xB,            // XREF: .rdata:snd_csv_entry_t const * const SND_ALIAS_FIELDS/s
    SND_CSV_SHORT = 0xC,
    SND_CSV_USHORT = 0xD,            // XREF: .rdata:snd_csv_entry_t const * const SND_ALIAS_FIELDS/s
    SND_CSV_NORM_BYTE = 0xE,            // XREF: .rdata:snd_csv_entry_t const * const SND_ALIAS_FIELDS/s
};

struct snd_csv_entry_t // sizeof=0x20
{                                       // XREF: .rdata:snd_csv_entry_t const * const SND_ALIAS_FIELDS/r
                                        // .rdata:snd_csv_entry_t const * const SND_ALIAS_STRINGS_FIELDS/r ...
    const char *name;
    unsigned int offset;
    bool required;
    // padding byte
    // padding byte
    // padding byte
    snd_csv_entry_type type;
    float minimum;
    float maximum;
    unsigned int length;
    const char **enum_ids;
};

void __cdecl SND_ParseInit(
                unsigned int groupCount,
                const snd_group *groups,
                unsigned int curveCount,
                const snd_curve *curves,
                unsigned int panCount,
                const snd_pan *pans,
                unsigned int snapshotGroupCount,
                const snd_snapshot_group *snapshotGroups);
void __cdecl SND_AliasGetFileName(const snd_alias_t *alias, char *filename, int size);


extern const snd_csv_entry_t *SND_TABLE_METADATA[9];
