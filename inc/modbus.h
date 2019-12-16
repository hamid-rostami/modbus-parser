#ifndef MODBUS_H_
#define MODBUS_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MODBUS_PARSER_VERSION_MAJOR 0
#define MODBUS_PARSER_VERSION_MINOR 1
#define MODBUS_PARSER_VERSION_PATCH 0

#define MODBUS_COIL_HIGH 0xFF00
#define MODBUS_COIL_LOW 0x0000

#define MODBUS_COILS_BYTE_LEN(qty) ((qty / 8) + ((qty % 8) > 0))

typedef struct modbus_parser modbus_parser;
typedef struct modbus_parser_settings modbus_parser_settings;

typedef int (*modbus_cb)(modbus_parser*);
typedef int (*modbus_data_cb)(modbus_parser*, uint8_t* at);

enum modbus_parser_type
{
  MODBUS_QUERY,
  MODBUS_RESPONSE
};

#define MODBUS_FUNC_MAP(XX)                                                    \
  XX(1, READ_COILS, "Read Coils")                                              \
  XX(2, READ_DISCRETE_IN, "Read Discrete Inputs")                              \
  XX(3, READ_HOLD_REG, "Read Holding Register")                                \
  XX(4, READ_IN_REG, "Read Input Register")                                    \
  XX(5, WRITE_COIL, "Wire Single Coil")                                        \
  XX(6, WRITE_REG, "Write Single Register")                                    \
  XX(15, WRITE_COILS, "Write Multiple Coils")                                  \
  XX(16, WRITE_REGS, "Write Miltiple Registers")

enum modbus_func
{
#define XX(num, name, string) MODBUS_FUNC_##name = num,
  MODBUS_FUNC_MAP(XX)
#undef XX
};

/*
#define MODBUS_ERRNO_MAP(XX)    \
  XX(CB_slave_addr, "the on_slave_addr callback failed")  \
  XX(CB_data_len, "the on_data_len callback failed")  \
  XX(CB_data_start, "the on_data_start callback failed")  \
  XX(CB_data_end, "the on_data_end callback failed")  \
  XX(CB_crc_error, "the on_crc_error callback failed")  \

#define XX(n, s) MBERR_##n,
enum modbus_errno {
  MODBUS_ERRNO_MAP(XX)
}
#undef XX
*/

enum modbus_parser_state
{
  s_slave_addr = 0,
  s_func,
  s_len,

  /* For Single reads */
  s_single_addr_hi,
  s_single_addr_lo,

  /* For Multiplie reads */
  s_start_addr_hi,
  s_start_addr_lo,

  /* Quantity */
  s_qty_hi,
  s_qty_lo,

  /* For Single reads */
  /*
  s_single_data_hi,
  s_single_data_lo,
  */

  /* For Multiple reads */
  s_data,

  /* NOTE: Don't edit order of these three elements,
   * It's important for updating CRC (inside parser state machine)
   */
  s_crc_lo,
  s_crc_hi,
  s_complete
};

struct modbus_parser
{
  /* PRIVATE */
  enum modbus_parser_type type;
  enum modbus_parser_state state;
  uint8_t data_cnt;
  uint8_t frame_start;
  uint16_t frame_crc; /* CRC inside frame */
  uint16_t calc_crc;  /* Calculated CRC */

  /* READ-ONLY */
  uint8_t slave_addr;
  enum modbus_func function;
  uint16_t addr;
  uint16_t qty;
  uint8_t data_len;
  const uint8_t* data;
  // bool crc_error;
  uint16_t errno;

  /* PUBLIC */
  void* arg;
};

struct modbus_parser_settings
{
  modbus_cb on_slave_addr;
  modbus_cb on_function;
  modbus_cb on_addr;
  modbus_cb on_qty;
  modbus_cb on_data_len;
  modbus_cb on_data_start;
  modbus_cb on_data_end;
  modbus_cb on_crc_error;
  modbus_cb on_complete;
};

struct modbus_query
{
  uint8_t slave_addr;
  enum modbus_func function;

  /* Address of register or coil. For MULTIPLE commands act as
   * Starting-Address
   */
  uint16_t addr;

  /* Quantity of registers or coils for MULTIPLE commands */
  uint16_t qty;

  /* Pointer to data to send.
   * For MODBUS_FUNC_WRITE_COILS command, least significant bit
   * addressing the lowest coil.
   */
  uint16_t* data;

  /* Length of data
   * NOTE: Don't get confused with byte-count. byte-count will calculate
   * with generator function.
   */
  uint8_t data_len;
};

void modbus_parser_init(modbus_parser* parser, enum modbus_parser_type t);

/* Initialize http_parser_settings members to 0
 */
void modbus_parser_settings_init(modbus_parser_settings* settings);

/* Executes the parser. Returns number of parsed bytes
 */
size_t modbus_parser_execute(modbus_parser* parser,
                             const modbus_parser_settings* settings,
                             const uint8_t* data,
                             size_t len);

/* Generate ready-to-send query and place it to buf array.
 * In success, return size of encoded message, otherwise return negative value
 */
int modbus_gen_query(struct modbus_query* q, uint8_t* buf, size_t sz);

void modbus_query_init(struct modbus_query* q);

const char* modbus_func_str(enum modbus_func f);

/* Calculate CRC from array of bytes */
uint16_t modbus_calc_crc(const uint8_t* data, size_t sz);

/* Update CRC with single byte, useful for calculating
 * CRC from streming bytes
 */
void modbus_crc_update(uint16_t* crc, uint8_t data);

#endif
