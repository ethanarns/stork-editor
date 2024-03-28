#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>
namespace Constants {
    // NitroSDK, crt0.c:193
    inline constexpr uint32_t SDK_NITROCODE_BE{0xdec00621};
    inline constexpr uint32_t ARM9_ROM_OFFSET{0x4000};
    inline constexpr uint32_t MAIN_MEM_OFFSET{0x0200'0000};

    // Universal Palette 0: 0x020d6f40
    inline constexpr uint32_t UNIVERSAL_PALETTE_0_ADDR{ 0x020d6f40};

    inline const char* NEW_BIN_FILE{"bin9_decomp_temp.bin"};
    inline const char* NEW_ROM_FILE{"rom_uncomp_temp.nds"};
    inline const char* CRSB_MAGIC{"CRSB"};
    inline const char* CRSB_EXTENSION{".crsb"};
    inline const char* CSCN_MAGIC{"CSCN"};
    inline const char* MPDZ_EXTENSION{".mpdz"};

    inline const char* WINDOW_TITLE{"Stork Editor"};

    inline constexpr uint32_t MPDZ_MAGIC_NUM{0x00544553}; // "SET "
    inline constexpr uint32_t SCEN_MAGIC_NUM{0x4e454353}; // "SCEN"
    inline constexpr uint32_t INFO_MAGIC_NUM{0x4f464e49}; // "INFO"
    inline constexpr uint32_t PLTB_MAGIC_NUM{0x42544c50}; // "PLTB"
    inline constexpr uint32_t PALB_MAGIC_NUM{0x424C4150}; // "PALB"
    inline constexpr uint32_t MPBZ_MAGIC_NUM{0x5a42504d}; // "MPBZ" / 4D 50 42 5A
    inline constexpr uint32_t COLZ_MAGIC_NUM{0x5a4c4f43}; // "COLZ" / 43 4F 4C 5A
    inline constexpr uint32_t IMGB_MAGIC_NUM{0x42474d49}; // "IMGB" / 49 4d 47 42
    inline constexpr uint32_t ANMZ_MAGIC_NUM{0x5a4d4e41}; // "ANMZ" / 41 4E 4D 5A
    inline constexpr uint32_t GRAD_MAGIC_NUM{0x44415247}; // "GRAD" / 47 52 41 44
    inline constexpr uint32_t SETD_MAGIC_NUM{0x44544553}; // "SETD"
    inline constexpr uint32_t ALPH_MAGIC_NUM{0x48504c41}; // "ALPH"
    inline constexpr uint32_t AREA_MAGIC_NUM{0x41455241}; // "AREA"
    inline constexpr uint32_t PATH_MAGIC_NUM{0x48544150}; // "PATH"
    inline constexpr uint32_t BLKZ_MAGIC_NUM{0x5a4b4c42}; // "BLKZ"
    inline constexpr uint32_t BRAK_MAGIC_NUM{0x4b415242}; // "BRAK"
    inline constexpr uint32_t PLAN_MAGIC_NUM{0x4e414c50}; // "PLAN"
    inline constexpr uint32_t IMBZ_MAGIC_NUM{0x5a424d49}; // "IMBZ"
    inline constexpr uint32_t SCRL_MAGIC_NUM{0x4c524353}; // "SCRL"
    inline constexpr uint32_t RAST_MAGIC_NUM{0x54534152}; // "RAST"
    inline constexpr uint32_t OBAR_MAGIC_NUM{0x5241424f}; // "OBAR" / 4F 42 41 52
    inline constexpr uint32_t OBJB_MAGIC_NUM{0x424A424F}; // "OBJB" / 4F 42 4A 42
    inline constexpr uint32_t OBJZ_MAGIC_NUM{0x5a4a424f}; // "OBJZ" / 4F 42 4A 5a
    inline constexpr uint32_t CSCN_MAGIC_NUM{0x4E435343}; // "CSCN" / 43 53 43 4E
    inline constexpr uint32_t CRSB_MAGIC_NUM{0x42535243}; // "CRSB" / 43 52 53 42
    inline constexpr uint32_t GINF_MAGIC_NUM{0x464e4947}; // "GINF"
    inline constexpr uint32_t GCOL_MAGIC_NUM{0x4c4f4347}; // "GCOL"

    inline constexpr int PALETTE_SIZE{0x20};
    inline constexpr int CHARTILE_DATA_SIZE{0x20};
    inline constexpr int16_t SINGLE_TILE_DIM{0x8};
    inline constexpr int EXTPAL_256_SIZE_BYTES(0x200); // 256 * 2
}

// Value pertaining to file-relative address (Starts at 0x0)
typedef uint32_t Address;
// Value pertaining to executable memory (0x02xxxxxx)
typedef uint32_t AddressMemory;
#endif
