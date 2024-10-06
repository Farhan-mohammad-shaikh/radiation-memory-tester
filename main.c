#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

#define NOR_CMD_WREN   0x06 
#define NOR_CMD_READ   0x03 
#define NOR_CMD_WRITE  0x02 
#define NOR_PAGE_SIZE 256

#define MY_SPI_MASTER DT_NODELABEL(my_spi_master)
#define MY_SPI_MASTER_CS_DT_SPEC SPI_CS_GPIOS_DT_SPEC_GET(DT_NODELABEL(reg_my_spi_master))

const struct device *spi_dev;

static void spi_init(void)
{
    spi_dev = DEVICE_DT_GET(MY_SPI_MASTER);
    if (!device_is_ready(spi_dev)) {
        printk("SPI master device not ready!\n");
    }
    struct gpio_dt_spec spim_cs_gpio = MY_SPI_MASTER_CS_DT_SPEC;
    if (!device_is_ready(spim_cs_gpio.port)) {
        printk("SPI master chip select device not ready!\n");
    }
}

static struct spi_config spi_cfg = {
    .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
    .frequency = 8000000,
    .slave = 0,
    .cs = {.gpio = MY_SPI_MASTER_CS_DT_SPEC, .delay = 0},
};

int spi_write_enable(void)
{
    uint8_t cmd = NOR_CMD_WREN;
    const struct spi_buf tx_buf = {.buf = &cmd, .len = sizeof(cmd)};
    const struct spi_buf_set tx_bufs = {.buffers = &tx_buf, .count = 1};

    return spi_write(spi_dev, &spi_cfg, &tx_bufs);
}

int spi_write_page(uint32_t address, const uint8_t *data, size_t size)
{
    int res;

	if (size > NOR_PAGE_SIZE) {
        printk("Error: Size exceeds page limit.\n");
        return -EINVAL;
    }

    uint8_t cmd_buf[4] = {
        NOR_CMD_WRITE,       // Write command
        (address >> 16) & 0xFF, // High byte of the address
        (address >> 8) & 0xFF,  // Middle byte of the address
        address & 0xFF        // Low byte of the address
    };

    struct spi_buf tx_bufs[2] = {
        {.buf = cmd_buf, .len = sizeof(cmd_buf)},
        {.buf = &data, .len = sizeof(data)}
    };

    struct spi_buf_set tx_buf_set = {.buffers = tx_bufs, .count = 2};

    res = spi_write_enable();
    if (res != 0) {
        printk("Failed to enable write operation: %d\n", res);
        return res;
    }

    res = spi_write(spi_dev, &spi_cfg, &tx_buf_set);
    if (res != 0) {
        printk("Failed to write data: %d\n", res);
    }

    return res;
}

int spi_read_data(uint32_t address, uint8_t *data, size_t size)
{
    int res;
    uint8_t cmd_buf[5] = {
        NOR_CMD_READ,        // Read command
        (address >> 16) & 0xFF, // High byte of the address
        (address >> 8) & 0xFF,  // Middle byte of the address
        address & 0xFF,         // Low byte of the address
        0x00                    // Dummy byte to give some time to process
    };

    const struct spi_buf tx_buf = {.buf = cmd_buf, .len = sizeof(cmd_buf)};
    const struct spi_buf_set tx_bufs = {.buffers = &tx_buf, .count = 1};

    uint8_t dummy_rx[size + 1]; // Buffer for received data +1 for the dummy byte
    struct spi_buf rx_buf = {.buf = dummy_rx, .len = sizeof(dummy_rx)};
    const struct spi_buf_set rx_bufs = {.buffers = &rx_buf, .count = 1};

    res = spi_transceive(spi_dev, &spi_cfg, &tx_bufs, &rx_bufs);

    if (res == 0) {
        memcpy(data, &dummy_rx[1], size); // Skip the first dummy byte
    }
    return res;
}

int main(void)
{
    int err;
    uint32_t start_address = 0x000000;  // Starting address of NOR Flash memory
    uint32_t end_address = 0xFFFFFF;    // Adjust this based on the size of your flash memory
      
	uint8_t write_buffer[NOR_PAGE_SIZE];               // Variable to store data for each address
              // Buffer to read data back
    uint32_t address;
	uint8_t read_data[4];  
	uint8_t expected_data;
    uint32_t step = 256; 
    printk("Starting NOR Flash SPI example\n");

    k_sleep(K_USEC(400));

    spi_init();
	
	for (int i = 0; i < NOR_PAGE_SIZE; i++) {
        write_buffer[i] = (uint8_t)(i & 0xFF);  // Example data
    }

    // Write data to the NOR Flash device
	 for (address = start_address; address <= end_address; address += NOR_PAGE_SIZE) {
        err = spi_write_page(address, write_buffer, NOR_PAGE_SIZE);
        if (err) {
            printk("Failed to write page at address 0x%X.\n", address);
            return err;
        }
		
    }

    printk("Successfully filled NOR Flash memory.\n");

	for (address = start_address; address <= end_address; address += step) {
        // Read data from the current address
        err = spi_read_data(address, read_data, sizeof(read_data));
        if (err) {
            printk("Failed to read data from address 0x%X.\n", address);
            return err;
        }

        // Calculate the expected data for the address (using the same pattern as the writing)
        expected_data = (uint8_t)(address & 0xFF);  // Example pattern: least significant byte of the address

        // Compare the read data with the expected data
        if (read_data[3] == expected_data) {
            printk("Success: Data at address 0x%X matches expected data 0x%02X.\n", address, expected_data);
        } else {
            printk("Error: Data mismatch at address 0x%X. Read 0x%02X, expected 0x%02X.\n", address, read_data[3], expected_data);
            return -EIO;
        }
    }

    printk("Sequential pattern read completed successfully.\n");

    return 0;
}

