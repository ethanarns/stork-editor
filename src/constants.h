#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>
#include <string>
namespace Constants {
    // NitroSDK, crt0.c:193
    inline constexpr uint32_t SDK_NITROCODE_BE{0xdec00621};
    inline constexpr uint32_t ARM9_ROM_OFFSET{0x4000};
    inline constexpr uint32_t MAIN_MEM_OFFSET{0x0200'0000};

    // Universal Palette 0: 0x020d6f40
    inline constexpr uint32_t UNIVERSAL_PALETTE_0_ADDR{ 0x020d6f40};

    inline const char* NEW_BIN_FILE{"bin9.bin"};
    inline const char* NEW_ROM_FILE{"rom_uncomp.nds"};
    inline const char* CRSB_MAGIC{"CRSB"};
    inline const char* CRSB_EXTENSION{".crsb"};
    inline const char* CSCN_MAGIC{"CSCN"};
    inline const char* MPDZ_EXTENSION{".mpdz"};

    inline constexpr uint32_t MPDZ_MAGIC_NUM{0x00544553}; // "SET "
    inline constexpr uint32_t SCEN_MAGIC_NUM{0x4e454353}; // "SCEN"
    inline constexpr uint32_t INFO_MAGIC_NUM{0x4f464e49}; // "INFO"
    inline constexpr uint32_t PLTB_MAGIC_NUM{0x42544c50}; // "PLTB"
    inline constexpr uint32_t MPBZ_MAGIC_NUM{0x5a42504d}; // "MPBZ" / 4D 50 42 5A
    inline constexpr uint32_t COLZ_MAGIC_NUM{0x5a4c4f43}; // "COLZ" / 43 4F 4C 5A
    inline constexpr uint32_t IMGB_MAGIC_NUM{0x42474d49}; // "IMGB" / 49 4d 47 42
    inline constexpr uint32_t ANMZ_MAGIC_NUM{0x5a4d4e41}; // "ANMZ" / 41 4E 4D 5A
    inline constexpr uint32_t GRAD_MAGIC_NUM{0x44415247}; // "GRAD" / 47 52 41 44
    inline constexpr uint32_t SETD_MAGIC_NUM{0x44544553}; // "SETD"

    inline constexpr int PALETTE_SIZE{0x20};
    inline constexpr int CHARTILE_DATA_SIZE{0x20};
}

// Value pertaining to file-relative address (Starts at 0x0)
typedef uint32_t Address;
// Value pertaining to executable memory (0x02xxxxxx)
typedef uint32_t AddressMemory;
#endif