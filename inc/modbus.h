#ifndef MODBUS_H_
#define MODBUS_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MODBUS_PARSER_VERSION_MAJOR 0
#define MODBUS_PARSER_VERSION_MINOR 1
#define MODBUS_PARSER_VERSION_PATCH 0

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
  XX(1, READ_COIL, Read - Coil)                                                \
  XX(2, READ_IN_STATUS, Read - Input - Status)                                 \
  XX(3, READ_HOLD_REG, Read - Holding - Register)                              \
  XX(4, READ_IN_REG, Read - Input - Register)                                  \
  XX(5, WRITE_COIL, Wire - Single - Coil)                                      \
  XX(6, WRITE_REG, Write - Single - Register)                                  \
  XX(15, WRITE_COILS, Write - Multiple - Coils)                                \
  XX(16, WRITE_REGS, Write - Miltiple - Registers)

enum modbus_func
{
#define XX(num, name, string) MODBUS_FUNC_##name = num,
  MODBUS_FUNC_MAP(XX)
#undef XX
};

struct modbus_parser
{
  /* PRIVATE */
  enum modbus_parser_type type;

  /* READ-ONLY */
  uint8_t slave_addr;
  enum modbus_func function;
  uint16_t start_addr;
  uint16_t nreg;
  uint16_t payload_len;
  uint8_t* payload;
  bool crc_error;
  /* TODO: error field */

  /* PUBLIC */
  void* data;
};

struct modbus_parser_settings
{
  modbus_cb on_frame_start;
  modbus_cb on_slave_addr;
  modbus_cb on_byte_count;
  modbus_cb on_start_data;
  modbus_cb on_end_data;
  modbus_cb on_crc_error;
};

void modbus_parser_init(modbus_parser* parser, enum modbus_parser_type t);

/* Initialize http_parser_settings members to 0
 */
void mosbus_parser_settings_init(modbus_parser_settings* settings);

/* Executes the parser. Returns number of parsed bytes
 */
size_t modbus_parser_execute(modbus_parser* parser,
                             const modbus_parser_settings* settings,
                             const uint8_t* data,
                             size_t len);

#endif
