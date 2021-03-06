
#include <stdio.h>
#include <math.h>
#include "driver/i2c.h"
#include "./ADXL343.h"

//Alphanumeric Display Defines
#define SLAVE_ADDR_ALPHA 0x62        // alphanumeric address
#define OSC 0x21                     // oscillator cmd
#define HT16K33_BLINK_DISPLAYON 0x01 // Display on cmd
#define HT16K33_BLINK_OFF 0          // Blink off cmd
#define HT16K33_BLINK_CMD 0x80       // Blink cmd
#define HT16K33_CMD_BRIGHTNESS 0xE0  // Brightness cmd

// Master I2C (LiderLite)
#define I2C_EXAMPLE_MASTER_SCL_IO 22        // gpio number for i2c clk
#define I2C_EXAMPLE_MASTER_SDA_IO 23        // gpio number for i2c data
#define I2C_EXAMPLE_MASTER_NUM I2C_NUM_0    // i2c port
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE 0 // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE 0 // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_FREQ_HZ 100000   // i2c master clock freq
#define WRITE_BIT I2C_MASTER_WRITE          // i2c master write
#define READ_BIT I2C_MASTER_READ            // i2c master read
#define ACK_CHECK_EN true                   // i2c master will check ack
#define ACK_CHECK_DIS false                 // i2c master will not check ack
#define ACK_VAL 0x00                        // i2c ack value
#define NACK_VAL 0xFF                       // i2c nack value

#define I2C_EXAMPLE_MASTER_SCL_IO_DISPLAY 15
#define I2C_EXAMPLE_MASTER_SDA_IO_DISPLAY 14
#define I2C_EXAMPLE_MASTER_NUM_DISPLAY I2C_NUM_1 // i2c port for alphanumeric

// ADXL343
#define SLAVE_ADDR 0x62 // 0x53

#define distance_high_reg 0x11
#define distance_low_reg 0x10
#define velocity_reg 0x09

//UART Defines
#define ECHO_TEST_TXD 17
#define ECHO_TEST_RXD 16
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

//UART Defines
#define ECHO_TEST_TXD_2 25
#define ECHO_TEST_RXD_2 26
#define ECHO_TEST_RTS_2 (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS_2 (UART_PIN_NO_CHANGE)

#define BUF_SIZE (128)

//Timer defines
#define TIMER_DIVIDER 16                             //  Hardware timer clock divider
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER) // convert counter value to seconds
#define TIMER_INTERVAL0_SEC (0.4)                    // sample test interval for the first timer
#define TIMER_INTERVAL1_SEC (5.78)                   // sample test interval for the second timer
#define TEST_WITHOUT_RELOAD 0                        // testing will be done without auto reload
#define TEST_WITH_RELOAD 1                           // testing will be done with auto reload

/////////////////////////////////////////
/////INSTANTIATING GLOBAL VARIABLES//////
/////////////////////////////////////////

//RANGE OF THE LIDARLITE
int16_t lidarLiteRange = 0;

///////////////////////////////////
/////TIMER FUNCTIONS!//////////////
///////////////////////////////////

/*
 * Timer group0 ISR handler
 *
 * Note:
 * We don't call the timer API here because they are not declared with IRAM_ATTR.
 * If we're okay with the timer irq not being serviced while SPI flash cache is disabled,
 * we can allocate this interrupt without the ESP_INTR_FLAG_IRAM flag and use the normal API.
 */

//////////////////////////////////////////////////////////////////////////
// Alphanumeric Functions ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//Hex values for the alphanumeric display numbers
const uint16_t FourteenSegmentASCII[10] = {
    0b000110000111111, /* 0 */
    0b000010000000110, /* 1 */
    0b000000011011011, /* 2 */
    0b000000010001111, /* 3 */
    0b000000011100110, /* 4 */
    0b010000001101001, /* 5 */
    0b000000011111101, /* 6 */
    0b000000000000111, /* 7 */
    0b000000011111111, /* 8 */
    0b000000011101111, /* 9 */
};

int alpha_oscillator()
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SLAVE_ADDR_ALPHA << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, OSC, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM_DISPLAY, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(200 / portTICK_RATE_MS);
    return ret;
}

// Set blink rate to off
int no_blink()
{
    int ret;
    i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();
    i2c_master_start(cmd2);
    i2c_master_write_byte(cmd2, (SLAVE_ADDR_ALPHA << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd2, HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (HT16K33_BLINK_OFF << 1), ACK_CHECK_EN);
    i2c_master_stop(cmd2);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM_DISPLAY, cmd2, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd2);
    vTaskDelay(200 / portTICK_RATE_MS);
    return ret;
}

// Set Brightness
int set_brightness_max(uint8_t val)
{
    int ret;
    i2c_cmd_handle_t cmd3 = i2c_cmd_link_create();
    i2c_master_start(cmd3);
    i2c_master_write_byte(cmd3, (SLAVE_ADDR_ALPHA << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd3, HT16K33_CMD_BRIGHTNESS | val, ACK_CHECK_EN);
    i2c_master_stop(cmd3);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM_DISPLAY, cmd3, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd3);
    vTaskDelay(200 / portTICK_RATE_MS);
    return ret;
}

////////////////////////////////////
////////I2C FUNCTIONS//////////////
////////////////////////////////////

// Function to initiate i2c -- note the MSB declaration!
// i2c communication is initialized here for both i2c devices
// Alphanumeric display and the LiderLite
static void i2c_master_init()
{
    // Debug
    printf("\n>> i2c Config\n");
    int err;

    // Port configuration
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;
    // Port configuration (alphanumeric)
    int i2c_master_port_display = I2C_EXAMPLE_MASTER_NUM_DISPLAY;

    /// Define I2C configurations for LidarLite
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;                        // Master mode
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;        // Default SDA pin
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;            // Internal pullup
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;        // Default SCL pin
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;            // Internal pullup
    conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ; // CLK frequency
    err = i2c_param_config(i2c_master_port, &conf);     // Configure
    if (err == ESP_OK)
    {
        printf("- parameters: ok\n");
    }

    // Install I2C driver
    err = i2c_driver_install(i2c_master_port, conf.mode,
                             I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                             I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
    if (err == ESP_OK)
    {
        printf("- initialized: yes\n");
    }

    /// Define I2C configurations for alphanumeric
    conf.mode = I2C_MODE_MASTER;                            // Master mode
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO_DISPLAY;    // Default SDA pin
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;                // Internal pullup
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO_DISPLAY;    // Default SCL pin
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;                // Internal pullup
    conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;     // CLK frequency
    err = i2c_param_config(i2c_master_port_display, &conf); // Configure
    if (err == ESP_OK)
    {
        printf("- parameters: ok\n");
    }

    // Install I2C driver
    err = i2c_driver_install(i2c_master_port_display, conf.mode,
                             I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                             I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
    if (err == ESP_OK)
    {
        printf("- initialized: yes\n");
    }

    // Data in MSB mode
    i2c_set_data_mode(i2c_master_port, I2C_DATA_MODE_MSB_FIRST, I2C_DATA_MODE_MSB_FIRST);

    // Data in MSB mode
    i2c_set_data_mode(i2c_master_port_display, I2C_DATA_MODE_MSB_FIRST, I2C_DATA_MODE_MSB_FIRST);
}

// Utility  Functions //////////////////////////////////////////////////////////

// Utility function to test for I2C device address -- not used in deploy
int testConnection(uint8_t devAddr, int32_t timeout)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (devAddr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    int err = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return err;
}

// Utility function to scan for i2c device
static void i2c_scanner()
{
    int32_t scanTimeout = 1000;
    printf("\n>> I2C scanning ..."
           "\n");
    uint8_t count = 0;
    for (uint8_t i = 1; i < 127; i++)
    {
        //printf("0x%X%s", i, "\n");
        if (testConnection(i, scanTimeout) == ESP_OK)
        {
            printf("- Device found at address: 0x%X%s", i, "\n");
            count++;
        }
    }
    if (count == 0)
    {
        printf("- No I2C devices found!"
               "\n");
    }
}

////////////////////////////////////////////////////////////////////////////////

// ADXL343 Functions ///////////////////////////////////////////////////////////

// Get Device ID
int getDeviceID(uint8_t *data)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, ADXL343_REG_DEVID, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, data, ACK_CHECK_DIS);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Write one byte to register
// Write one byte to register
int writeRegister(uint8_t reg, uint8_t data)
{

    //printf("--Writing %d to reg %d!--\n", data, reg);

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return 0;
}

// Read register
uint8_t readRegister(uint8_t reg, uint8_t *regData)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, regData, ACK_CHECK_DIS);

    //printf("--Read in value: *regData = %d\n", *regData);

    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    //printf("---Read in value: *regData = %d\n", *regData);
    return 0;
}

// read 16 bits (2 bytes)
// Function used to read the LidarLite according to the garmin data sheet
uint16_t read16Dis()
{
    uint8_t data = 0;
    uint8_t data2 = 0;
    uint16_t overall = 0;

    // uint8_t reg1 = 0x01;
    uint8_t reg2 = 0x00;
    uint8_t reg3 = 0x04;
    uint8_t reg4 = 0x10;
    uint8_t reg5 = 0x01;
    uint8_t reg6 = 0x11;

    writeRegister(reg2, reg3);
    readRegister(reg5, &data);
    //printf("%d\n", data);

    while (readRegister(reg5, &data) % 2 == 1)
    {
        //   readRegister(reg5, &data);
    }

    readRegister(reg4, &data);
    readRegister(reg6, &data2);

    overall = (data2 << 8) + data;
    return overall;
}

////////////////////////////////////////////////////////////////////////////////

void getDistance(float *dis)
{
    lidarLiteRange = read16Dis();
    printf("LIDARLite: %.2d  centimeters\n", lidarLiteRange);
}

// Task to continuously poll acceleration and calculate roll and pitch
static void test_lidarLite()
{
    printf("\n>> Polling LidarLite\n");
    while (1)
    {
        float output;
        vTaskDelay(400 / portTICK_RATE_MS);
        getDistance(&output);
    }
}

void app_main(void)
{
    i2c_master_init();
    i2c_scanner();

    xTaskCreate(test_lidarLite, "test_lidarLite", 4096, NULL, 5, NULL);
}
