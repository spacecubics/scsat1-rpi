#pragma once

#include <iostream>
#include <vector>

#define UNKNOWN 0xFFFFFFFE
#define DEFAULT(x, def) (static_cast<uint32_t>(x) == UNKNOWN || static_cast<uint32_t>(x) == static_cast<uint32_t>(-1)) ? (def):(x)
namespace Arducam {

enum {
    EXPOSURE_DELAY=0,
    GAIN_DELAY=1,
    VBLANK_DELAY=2,
    HIDE_FRAMES=3,
};

enum I2C_MODE {
    I2C_MODE_8_8 = 0x11,
    I2C_MODE_8_16 = 0x12,
    I2C_MODE_16_8 = 0x21,
    I2C_MODE_16_16 = 0x22,
    I2C_MODE_BLOCK,
};

typedef struct {
    int device_handle;
    enum I2C_MODE i2c_mode;
    uint16_t chip_address;
    uint16_t register_address;
    uint32_t register_value;
} I2C_RW_STATE;

class Device {
public:
    Device();
    ~Device();
    bool init(int bus = 0);
    bool probe();
    std::vector<uint8_t> get_tuning_data();
    int get_info(uint32_t id);
    uint32_t get_version();
    int set_tuning_data(uint8_t *firmware_data, size_t data_size);

private:
    uint32_t calcCheckSum(unsigned char *data, int length);
    int i2c_write_data(I2C_RW_STATE *state, uint8_t *data, uint16_t length);
    int i2c_read_data(I2C_RW_STATE *state, uint8_t *data, uint16_t length);
    int write32(I2C_RW_STATE *state);
    int read32(I2C_RW_STATE *state);

    int i2c_read_reg(I2C_RW_STATE *state, uint8_t *data, uint16_t length);

private:
    I2C_RW_STATE state_;
};

class ArducamUtils
{
public:
    ArducamUtils(int bus = 0);
    ~ArducamUtils();
    std::string readJson();
    int readInfo(uint32_t id);
    uint32_t getVersion();

    int writeJson(std::string tuning_data);
    int convert(std::string &tuning_data);
    static int convertPISP(std::string &pisp_tuning_data);

protected:
    std::string decompress(uint8_t *data, uint32_t length);
    int compression(std::string json, std::vector<uint8_t> &buffer);

private:
    Device *dev_;

};

} // namespace Arducam
