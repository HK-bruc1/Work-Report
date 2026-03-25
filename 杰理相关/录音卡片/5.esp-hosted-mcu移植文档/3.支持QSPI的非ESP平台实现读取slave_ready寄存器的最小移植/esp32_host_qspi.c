#include "esp32_host_qspi.h"

#include <stddef.h>

/* ---------------- 直接对齐 ESP-Hosted 协议的常量 ---------------- */
#define ESP32_HOST_QSPI_CMD_RDBUF_BASE   0x02
#define ESP32_HOST_QSPI_CMD_WRBUF_BASE   0x01
#define ESP32_HOST_QSPI_CMD_RDDMA_BASE   0x04
#define ESP32_HOST_QSPI_CMD_WRDMA_BASE   0x03
#define ESP32_HOST_QSPI_CMD_CMD9_BASE    0x09

#define ESP32_HOST_QSPI_MASK_2BIT        0x50
#define ESP32_HOST_QSPI_MASK_4BIT        0xA0

#define ESP32_HOST_QSPI_DUMMY_BITS_FIXED 8

typedef struct {
    esp32_host_qspi_cfg_t cfg;
    esp32_host_qspi_lock_ops_t lock_ops;
    esp32_host_qspi_lines_t cur_lines;
    bool inited;
} esp32_host_qspi_ctx_t;

static esp32_host_qspi_ctx_t s_ctx;

static uint8_t prv_mask_from_lines(esp32_host_qspi_lines_t lines)
{
    return (lines == ESP32_HOST_QSPI_LINES_4) ? ESP32_HOST_QSPI_MASK_4BIT : ESP32_HOST_QSPI_MASK_2BIT;
}

static void prv_lock_if_needed(bool lock_required)
{
    if (lock_required && s_ctx.lock_ops.lock) {
        s_ctx.lock_ops.lock();
    }
}

static void prv_unlock_if_needed(bool lock_required)
{
    if (lock_required && s_ctx.lock_ops.unlock) {
        s_ctx.lock_ops.unlock();
    }
}

/*
 * ====================== 平台适配区（最关键） ======================
 *
 * 这里不是“随便补几个收发 API”就够了。
 * 你必须确认平台底层能够表达官方要求的单事务多 phase 结构。
 *
 * 如果底层只能做到：
 * 1. 发送一个字节
 * 2. 收发一段 DMA
 * 3. 在阶段之间切整个总线线宽
 *
 * 那么这套模板虽然形式上能填完，实际也大概率无法成功。
 */

static int platform_qspi_hw_init(const esp32_host_qspi_cfg_t *cfg)
{
    (void)cfg;
    /*
     * [平台适配-必须实现]
     * 1. 初始化 QSPI / SPI HD 控制器
     * 2. 固定 mode=0, cmd_bits=8, addr_bits=8, dummy_bits=8
     * 3. 初始化阶段先切到 2 线
     * 4. 允许半双工收发
     */
    return -1;
}

static int platform_qspi_hw_deinit(void)
{
    return -1;
}

static int platform_qspi_set_lines(esp32_host_qspi_lines_t lines)
{
    (void)lines;
    /*
     * [平台适配-必须实现]
     * 切换后续事务 addr/data 阶段的线数。
     *
     * 注意：
     * 这不是“在阶段之间切整个 SPI 外设模式”这么简单，
     * 而是要保证最终产出的事务仍然符合官方 phase 结构。
     */
    return -1;
}

static int platform_qspi_xfer_reg_read(uint8_t cmd, uint8_t reg, uint8_t dummy_bits, uint32_t *out_data)
{
    (void)cmd;
    (void)reg;
    (void)dummy_bits;
    (void)out_data;
    /*
     * [平台适配-最关键]
     * 必须在一笔连续事务内完成：
     *   cmd(8bits,1线) -> addr(8bits,2/4线) -> dummy(8bits,1线) -> data(32bits,2/4线)
     *
     * 如果这里的实现只是：
     * 1. 发 cmd
     * 2. 切总线位宽
     * 3. 再发 addr
     * 4. 再切回去发 dummy
     * 5. 最后再 DMA 收 data
     *
     * 那么“逻辑上像”官方事务，但“控制器语义上”未必等价，
     * 实际上很可能无法被 ESP Slave 识别。
     */
    return -1;
}

static int platform_qspi_xfer_reg_write(uint8_t cmd, uint8_t reg, uint8_t dummy_bits, uint32_t data)
{
    (void)cmd;
    (void)reg;
    (void)dummy_bits;
    (void)data;
    /*
     * [平台适配-必须实现]
     * 与读寄存器同理，必须保证：
     *   cmd -> addr -> dummy -> data
     * 仍是一笔官方语义上的连续事务。
     */
    return -1;
}

static int platform_qspi_send_cmd_only(uint8_t cmd)
{
    (void)cmd;
    /*
     * [平台适配-必须实现]
     * 只发送命令 phase，例如 CMD9。
     * 不能错误带出额外 addr/data phase。
     */
    return -1;
}

/* ====================== 平台适配区结束 ====================== */

int esp32_host_qspi_init(const esp32_host_qspi_cfg_t *cfg,
                         const esp32_host_qspi_lock_ops_t *lock_ops)
{
    if (!cfg) {
        return -1;
    }

    if (cfg->cmd_bits != 8 || cfg->addr_bits != 8 || cfg->dummy_bits != ESP32_HOST_QSPI_DUMMY_BITS_FIXED) {
        return -1;
    }

    if (cfg->lines_init != ESP32_HOST_QSPI_LINES_2 && cfg->lines_init != ESP32_HOST_QSPI_LINES_4) {
        return -1;
    }

    s_ctx.cfg = *cfg;
    s_ctx.cur_lines = ESP32_HOST_QSPI_LINES_2;
    s_ctx.inited = false;

    if (lock_ops) {
        s_ctx.lock_ops = *lock_ops;
    } else {
        s_ctx.lock_ops.lock = NULL;
        s_ctx.lock_ops.unlock = NULL;
    }

    if (platform_qspi_hw_init(&s_ctx.cfg) != 0) {
        return -1;
    }

    if (platform_qspi_set_lines(ESP32_HOST_QSPI_LINES_2) != 0) {
        (void)platform_qspi_hw_deinit();
        return -1;
    }

    s_ctx.inited = true;
    return 0;
}

int esp32_host_qspi_deinit(void)
{
    if (!s_ctx.inited) {
        return 0;
    }
    s_ctx.inited = false;
    return platform_qspi_hw_deinit();
}

int esp32_host_qspi_read_reg(uint8_t reg, uint32_t *data, bool lock_required)
{
    int ret;
    uint8_t cmd;

    if (!s_ctx.inited || !data) {
        return -1;
    }

    cmd = (uint8_t)(ESP32_HOST_QSPI_CMD_RDBUF_BASE | prv_mask_from_lines(s_ctx.cur_lines));

    prv_lock_if_needed(lock_required);
    ret = platform_qspi_xfer_reg_read(cmd, reg, ESP32_HOST_QSPI_DUMMY_BITS_FIXED, data);
    prv_unlock_if_needed(lock_required);

    return ret;
}

int esp32_host_qspi_write_reg(uint8_t reg, uint32_t data, bool lock_required)
{
    int ret;
    uint8_t cmd;

    if (!s_ctx.inited) {
        return -1;
    }

    cmd = (uint8_t)(ESP32_HOST_QSPI_CMD_WRBUF_BASE | prv_mask_from_lines(s_ctx.cur_lines));

    prv_lock_if_needed(lock_required);
    ret = platform_qspi_xfer_reg_write(cmd, reg, ESP32_HOST_QSPI_DUMMY_BITS_FIXED, data);
    prv_unlock_if_needed(lock_required);

    return ret;
}

int esp32_host_qspi_set_data_lines(esp32_host_qspi_lines_t lines)
{
    if (!s_ctx.inited) {
        return -1;
    }
    if (lines != ESP32_HOST_QSPI_LINES_2 && lines != ESP32_HOST_QSPI_LINES_4) {
        return -1;
    }
    if (platform_qspi_set_lines(lines) != 0) {
        return -1;
    }
    s_ctx.cur_lines = lines;
    return 0;
}

int esp32_host_qspi_send_cmd9(bool lock_required)
{
    uint8_t cmd;
    int ret;

    if (!s_ctx.inited) {
        return -1;
    }

    cmd = (uint8_t)(ESP32_HOST_QSPI_CMD_CMD9_BASE | prv_mask_from_lines(s_ctx.cur_lines));

    prv_lock_if_needed(lock_required);
    ret = platform_qspi_send_cmd_only(cmd);
    prv_unlock_if_needed(lock_required);

    return ret;
}

int esp32_host_qspi_read_dma(uint8_t *data, uint16_t size, bool lock_required)
{
    (void)data;
    (void)size;
    (void)lock_required;
    /*
     * [完整移植阶段]
     * 后续需要补齐 RDDMA + CMD8 等完整流程。
     * 但如果最小寄存器事务都不满足官方结构，这里继续做也没有意义。
     */
    return -1;
}

int esp32_host_qspi_write_dma(const uint8_t *data, uint16_t size, bool lock_required)
{
    (void)data;
    (void)size;
    (void)lock_required;
    /*
     * [完整移植阶段]
     * 后续需要补齐 WRDMA + WR_DONE 等完整流程。
     * 同样，必须建立在最小寄存器事务已真正跑通的前提下。
     */
    return -1;
}

int esp32_host_qspi_read_slave_ready(uint32_t *data, int poll, bool lock_required)
{
    int ret = -1;
    int i;

    if (!data) {
        return -1;
    }

    for (i = 0; i <= poll; i++) {
        ret = esp32_host_qspi_read_reg((uint8_t)ESP32_HOST_QSPI_REG_SLAVE_READY, data, lock_required);
        if (ret) {
            return ret;
        }
    }

    return 0;
}

int esp32_host_qspi_open_datapath(bool lock_required)
{
    return esp32_host_qspi_write_reg((uint8_t)ESP32_HOST_QSPI_REG_SLAVE_CTRL,
                                     (uint32_t)ESP32_HOST_QSPI_CTRL_DATAPATH_ON,
                                     lock_required);
}
