#ifndef ESP32_HOST_QSPI_H
#define ESP32_HOST_QSPI_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 这是“非 ESP Host 平台”对接 ESP-Hosted SPI HD 的最小模板。
 *
 * 重要说明：
 * 1. 这是协议接口模板，不代表当前平台一定具备官方要求的事务能力。
 * 2. 如果你的平台只能“发字节 + 切总线线宽 + DMA 收发”，通常不足以实现官方 SPI HD 事务。
 * 3. 只有当底层控制器支持在单事务内表达 cmd/addr/dummy/data 多阶段结构时，才有较高成功概率。
 */

typedef enum {
    ESP32_HOST_QSPI_LINES_2 = 2,
    ESP32_HOST_QSPI_LINES_4 = 4,
} esp32_host_qspi_lines_t;

typedef struct {
    uint32_t spi_mode;       /* 一般为 SPI Mode 0 */
    uint32_t clock_hz;       /* 建议起步 1~5MHz */
    uint8_t  addr_bits;      /* 官方固定 8 */
    uint8_t  cmd_bits;       /* 官方固定 8 */
    uint8_t  dummy_bits;     /* 官方固定 8 */
    esp32_host_qspi_lines_t lines_init; /* 初始化阶段应先走 2 线 */
} esp32_host_qspi_cfg_t;

typedef struct {
    void (*lock)(void);
    void (*unlock)(void);
} esp32_host_qspi_lock_ops_t;

/*
 * 官方协议关键寄存器：
 * 1. SLAVE_READY = 0x00，期望值 0xEE
 * 2. SLAVE_CTRL  = 0x14，DATAPATH_ON = 0x01
 *
 * 注意：
 * 本模板已经按当前项目实测结果同步为 0x14。
 */
typedef enum {
    ESP32_HOST_QSPI_REG_SLAVE_READY   = 0x00,
    ESP32_HOST_QSPI_REG_SLAVE_CTRL    = 0x14,
} esp32_host_qspi_reg_t;

typedef enum {
    ESP32_HOST_QSPI_STATE_SLAVE_READY = 0xEE,
} esp32_host_qspi_state_t;

typedef enum {
    ESP32_HOST_QSPI_CTRL_DATAPATH_ON  = (1u << 0),
} esp32_host_qspi_ctrl_t;

int esp32_host_qspi_init(const esp32_host_qspi_cfg_t *cfg,
                         const esp32_host_qspi_lock_ops_t *lock_ops);

int esp32_host_qspi_deinit(void);

/*
 * 读寄存器：
 * 必须在单次事务内完成：
 *   cmd(1线) -> addr(2/4线) -> dummy(1线) -> data(2/4线)
 *
 * 如果底层只能靠多次普通 API 拼接，这里大概率无法满足官方事务要求。
 */
int esp32_host_qspi_read_reg(uint8_t reg, uint32_t *data, bool lock_required);

/*
 * 写寄存器：
 * 用于写 SLAVE_CTRL = 0x01 打开 datapath。
 */
int esp32_host_qspi_write_reg(uint8_t reg, uint32_t data, bool lock_required);

/*
 * 能力协商后切换后续事务的数据线数。
 * 注意：初始化阶段仍建议固定先走 2 线。
 */
int esp32_host_qspi_set_data_lines(esp32_host_qspi_lines_t lines);

/*
 * 发送 CMD9。
 * 该事务应为“仅命令 phase”，不能错误带出额外 addr/data phase。
 */
int esp32_host_qspi_send_cmd9(bool lock_required);

/*
 * DMA 通道接口保留给完整移植阶段。
 * 最小移植阶段只需先关注寄存器读写是否真的符合官方事务结构。
 */
int esp32_host_qspi_read_dma(uint8_t *data, uint16_t size, bool lock_required);
int esp32_host_qspi_write_dma(const uint8_t *data, uint16_t size, bool lock_required);

/*
 * 便于最小验证阶段直接调用的辅助接口。
 */
int esp32_host_qspi_read_slave_ready(uint32_t *data, int poll, bool lock_required);
int esp32_host_qspi_open_datapath(bool lock_required);

#ifdef __cplusplus
}
#endif

#endif /* ESP32_HOST_QSPI_H */
