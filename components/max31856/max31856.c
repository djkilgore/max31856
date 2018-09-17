#include "max31856.h"

const char *TAG = "MAX31856";

void max31856_write_register(spi_device_handle_t spi_handle, uint8_t address, uint8_t data) {
    esp_err_t ret;
    spi_transaction_t spi_transaction;
    memset( &spi_transaction, 0, sizeof( spi_transaction_t ) );
    uint8_t tx_data[1] = {address | 0x80};

    gpio_set_level(PIN_NUM_CS, 0);
    spi_transaction.flags = SPI_TRANS_USE_RXDATA;
    spi_transaction.length = 8;
    spi_transaction.tx_buffer = tx_data;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);

    tx_data[0] = data;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);   
    gpio_set_level(PIN_NUM_CS, 1);
}

uint8_t max31856_read_register(spi_device_handle_t spi_handle, uint8_t address) {
    esp_err_t ret;
    spi_transaction_t spi_transaction;
    memset( &spi_transaction, 0, sizeof( spi_transaction_t ) );
    uint8_t tx_data[1] = {address & 0x7F};

    gpio_set_level(PIN_NUM_CS, 0);
    spi_transaction.flags = SPI_TRANS_USE_RXDATA;
    spi_transaction.length = 8;
    spi_transaction.tx_buffer = tx_data;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);

    tx_data[0] = 0xFF;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);
    gpio_set_level(PIN_NUM_CS, 1);
    uint8_t reg_value = spi_transaction.rx_data[0];
    return reg_value;
}

uint16_t max31856_read_register16(spi_device_handle_t spi_handle, uint8_t address) {
    esp_err_t ret;
    spi_transaction_t spi_transaction;
    memset( &spi_transaction, 0, sizeof( spi_transaction_t ) );
    uint8_t tx_data[1] = {address & 0x7F};

    gpio_set_level(PIN_NUM_CS, 0);
    spi_transaction.length = 8;
    spi_transaction.flags = SPI_TRANS_USE_RXDATA;
    spi_transaction.tx_buffer = tx_data;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);

    tx_data[0] = 0xFF;
    spi_transaction.length = 8;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);
    uint8_t b1 = spi_transaction.rx_data[0];

    spi_transaction.length = 8;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);
    uint8_t b2 = spi_transaction.rx_data[0];
    gpio_set_level(PIN_NUM_CS, 1);

    uint16_t reg_value = ((b1 << 8) | b2);
    return reg_value;
}

uint32_t max31856_read_register24(spi_device_handle_t spi_handle, uint8_t address) {
    esp_err_t ret;
    spi_transaction_t spi_transaction;
    memset( &spi_transaction, 0, sizeof( spi_transaction_t ) );
    uint8_t tx_data[1] = {address & 0x7F};

    gpio_set_level(PIN_NUM_CS, 0);
    spi_transaction.length = 8;
    spi_transaction.flags = SPI_TRANS_USE_RXDATA;
    spi_transaction.tx_buffer = tx_data;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);

    tx_data[0] = 0xFF;
    spi_transaction.length = 8;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);
    uint8_t b1 = spi_transaction.rx_data[0];

    tx_data[0] = 0xFF;
    spi_transaction.length = 8;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);
    uint8_t b2 = spi_transaction.rx_data[0];

    tx_data[0] = 0xFF;
    spi_transaction.length = 8;
    ret = spi_device_transmit(spi_handle, &spi_transaction);
    ESP_ERROR_CHECK(ret);
    uint8_t b3 = spi_transaction.rx_data[0];

    uint32_t reg_value = ((b1 << 16) | (b2 << 8) | b3);
    return reg_value;
}

void max31856_oneshot_temperature(spi_device_handle_t spi_handle) {
    max31856_write_register(spi_handle, MAX31856_CJTO_REG, 0x00);
    uint8_t val = max31856_read_register(spi_handle, MAX31856_CR0_REG);
    val &= ~MAX31856_CR0_AUTOCONVERT;
    val |= MAX31856_CR0_1SHOT;
    max31856_write_register(spi_handle, MAX31856_CR0_REG, val);
    vTaskDelay(250 / portTICK_PERIOD_MS);
}

void thermocouple_set_type(max31856_cfg_t *max31856, max31856_thermocoupletype_t tc_type) {
    spi_device_handle_t spi = max31856->spi;
    uint8_t val = max31856_read_register(spi, MAX31856_CR1_REG);
    val &= 0xF0; // Mask off bottom 4 bits
    val |= (uint8_t)tc_type & 0x0F;
    max31856_write_register(spi, MAX31856_CR1_REG, val);
}

max31856_thermocoupletype_t thermocouple_get_type(max31856_cfg_t *max31856) {
    spi_device_handle_t spi = max31856->spi;
    uint8_t val = max31856_read_register(spi, MAX31856_CR1_REG);
    val &= 0x0F;

    switch (val) {
        case MAX31856_TCTYPE_B: ESP_LOGI(TAG, "TC Type: B"); break;
        case MAX31856_TCTYPE_E: ESP_LOGI(TAG, "TC Type: E"); break;
        case MAX31856_TCTYPE_J: ESP_LOGI(TAG, "TC Type: J"); break;
        case MAX31856_TCTYPE_K: ESP_LOGI(TAG, "TC Type: K"); break;
        case MAX31856_TCTYPE_N: ESP_LOGI(TAG, "TC Type: N"); break;
        case MAX31856_TCTYPE_R: ESP_LOGI(TAG, "TC Type: R"); break;
        case MAX31856_TCTYPE_S: ESP_LOGI(TAG, "TC Type: S"); break;
        case MAX31856_TCTYPE_T: ESP_LOGI(TAG, "TC Type: T"); break;
        case MAX31856_VMODE_G8: ESP_LOGI(TAG, "Voltage x8 Gain mode"); break;
        case MAX31856_VMODE_G32: ESP_LOGI(TAG, "Voltage x8 Gain mode"); break;
        default: ESP_LOGI(TAG, "TC Type: Unknown"); break;
    }

    return (max31856_thermocoupletype_t)(val);
}

uint8_t thermocouple_read_fault(max31856_cfg_t *max31856) {
    spi_device_handle_t spi = max31856->spi;
    uint8_t fault_val = max31856_read_register(spi, MAX31856_SR_REG);
    if (fault_val) {
        switch(fault_val) {
            case MAX31856_FAULT_CJRANGE: ESP_LOGI(TAG, "Fault: Cold Junction Range"); break;
            case MAX31856_FAULT_TCRANGE: ESP_LOGI(TAG, "Fault: Thermocouple Range"); break;
            case MAX31856_FAULT_CJHIGH: ESP_LOGI(TAG, "Fault: Cold Junction High"); break;
            case MAX31856_FAULT_CJLOW: ESP_LOGI(TAG, "Fault: Cold Junction Low"); break;
            case MAX31856_FAULT_TCHIGH: ESP_LOGI(TAG, "Fault: Thermocouple High"); break;
            case MAX31856_FAULT_TCLOW: ESP_LOGI(TAG, "Fault: Thermocouple Low"); break;
            case MAX31856_FAULT_OVUV: ESP_LOGI(TAG, "Fault: Over/Under Voltage"); break;
            case MAX31856_FAULT_OPEN: ESP_LOGI(TAG, "Fault: Thermocouple Open"); break;
        }
    }
    max31856->fault = fault_val;
    return fault_val;
}

float thermocouple_read_coldjunction(max31856_cfg_t *max31856) {
    spi_device_handle_t spi = max31856->spi;
    max31856_oneshot_temperature(spi);
    uint16_t cj_temp = max31856_read_register16(spi, MAX31856_CJTH_REG);
    float cj_temp_float = cj_temp;
    cj_temp_float /= 256.0;
    max31856->coldjunction_c = cj_temp_float;
    max31856->coldjunction_f = (1.8 * cj_temp_float) + 32.0;
    return cj_temp_float;
}

float thermocouple_read_temperature(max31856_cfg_t *max31856) {
    spi_device_handle_t spi = max31856->spi;
    max31856_oneshot_temperature(spi);
    uint32_t tc_temp = max31856_read_register24(spi, MAX31856_LTCBH_REG);
    if (tc_temp & 0x800000) {
        tc_temp |= 0xFF000000; // fix sign bit
    }
    tc_temp >>= 5;  // bottom 5 bits are unused
    float tc_temp_float = tc_temp;
    tc_temp_float *= 0.0078125;
    max31856->thermocouple_c = tc_temp_float;
    max31856->thermocouple_f = (1.8 * tc_temp_float) + 32.0;
    return tc_temp_float;
}

max31856_cfg_t max31856_init() {
    ESP_LOGI(TAG, "Initialize");
    static spi_device_handle_t spi;

    gpio_config_t io_conf;
    io_conf.pull_down_en = 0;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL<<PIN_NUM_CS);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    gpio_set_level(PIN_NUM_CS, 1);

    vTaskDelay(100 / portTICK_PERIOD_MS);

    esp_err_t ret;
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = (APB_CLK_FREQ/10), // 8 Mhz
        .dummy_bits = 0,
        .mode = 1,
        .flags = 0,
        .spics_io_num = -1, // Manually Control CS
        .queue_size = 1,
    };

    ret=spi_bus_initialize(HSPI_HOST, &buscfg, 0); // No DMA
    ESP_ERROR_CHECK(ret);
    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

    // Assert on All Faults
    max31856_write_register(spi, MAX31856_MASK_REG, 0x00);

    // Open Circuit Detection
    max31856_write_register(spi, MAX31856_CR0_REG, MAX31856_CR0_OCFAULT0);

    return (max31856_cfg_t){
        .spi = spi,
    };
}