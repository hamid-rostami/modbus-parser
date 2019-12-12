#include "modbus.h"
#include <assert.h>
#include <stdio.h>

#define TEST_START() printf("<< %s STARTED >>\n", __func__)
#define TEST_SUCCESS() printf("** %s SUCCESS **\n\n", __func__)

#define HEX_FMT "0x%04X (%d)"
#define HEX(n) n, n

#define UINT16(p) ((*(&p) << 8) + (*(&p + 1)))

#define ADD_CRC(buf)                                                           \
  do {                                                                         \
    uint16_t crc = modbus_calc_crc(buf, sizeof(buf) - 2);                      \
    buf[sizeof(buf) - 1] = crc >> 8;                                           \
    buf[sizeof(buf) - 2] = crc & 0x00FF;                                       \
  } while (0)

int
on_slave_addr(struct modbus_parser* p)
{
  printf("Slave addr: " HEX_FMT "\n", HEX(p->slave_addr));
  return 0;
}

int
on_function(struct modbus_parser* p)
{
  printf("Function: %d (%s)\n", p->function, modbus_func_str(p->function));
  return 0;
}

int
on_addr(struct modbus_parser* p)
{
  printf("Address: " HEX_FMT "\n", HEX(p->addr));
  return 0;
}

int
on_qty(struct modbus_parser* p)
{
  printf("Quantity: " HEX_FMT "\n", HEX(p->qty));
  return 0;
}

int
on_data_len(struct modbus_parser* p)
{
  printf("Data len: %d\n", p->data_len);
  return 0;
}

int
on_data_start(struct modbus_parser* p)
{
  printf("Data start\n");
  return 0;
}

int
on_data_end(struct modbus_parser* p)
{
  int n = 0;

  printf("Data:\n");

  for (int i = 0; i < p->data_len; i++) {
    n += printf("%02X ", *(p->data + i));
    if (n > 70) {
      n = 0;
      printf("\n");
    }
  }
  printf("\n");
  return 0;
}

int
on_crc_error(struct modbus_parser* p)
{
  printf("CRC Error, 0x%04X != 0x%04X\n", p->frame_crc, p->calc_crc);
  return 0;
}

int
on_complete(struct modbus_parser* p)
{
  printf("Complete\n");
  return 0;
}

void
test_read_coils(struct modbus_parser* parser,
                struct modbus_parser_settings* settings)
{
  uint8_t res[] = { 0x11, MODBUS_FUNC_READ_COILS, 0x03, 0xCD, 0x6B, 0x05, 0x00,
                    0x00 };
  size_t n;

  TEST_START();

  ADD_CRC(res);
  modbus_parser_init(parser, MODBUS_RESPONSE);
  n = modbus_parser_execute(parser, settings, res, sizeof(res));

  assert(n == sizeof(res));
  assert(parser->slave_addr == res[0]);
  assert(parser->function == res[1]);
  assert(parser->data_len == res[2]);
  TEST_SUCCESS();
}

void
test_read_discrete_in(struct modbus_parser* parser,
                      struct modbus_parser_settings* settings)
{
  uint8_t res[] = {
    0x11, MODBUS_FUNC_READ_DISCRETE_IN, 0x03, 0xAC, 0xDB, 0x35, 0x00, 0x00
  };
  size_t n;

  TEST_START();

  ADD_CRC(res);
  modbus_parser_init(parser, MODBUS_RESPONSE);

  n = modbus_parser_execute(parser, settings, res, sizeof(res));

  assert(n == sizeof(res));
  assert(parser->slave_addr == res[0]);
  assert(parser->function == res[1]);
  assert(parser->data_len == res[2]);

  TEST_SUCCESS();
}

void
test_read_hold_reg(struct modbus_parser* parser,
                   struct modbus_parser_settings* settings)
{
  uint8_t res[] = { 0x11, MODBUS_FUNC_READ_HOLD_REG,
                    0x06, 0x02,
                    0x2B, 0x00,
                    0x00, 0x00,
                    0x64, 0x00,
                    0x00 };
  size_t n;

  TEST_START();

  ADD_CRC(res);
  modbus_parser_init(parser, MODBUS_RESPONSE);

  /* Parse first 3 bytes */
  n = modbus_parser_execute(parser, settings, res, 3);
  /* Parse res of bytes */
  n += modbus_parser_execute(parser, settings, res + 3, sizeof(res) - 3);

  assert(n == sizeof(res));
  assert(parser->slave_addr == res[0]);
  assert(parser->function == res[1]);
  assert(parser->data_len == res[2]);

  TEST_SUCCESS();
}

void
test_read_in_reg(struct modbus_parser* parser,
                 struct modbus_parser_settings* settings)
{
  uint8_t res[] = {
    0x11, MODBUS_FUNC_READ_IN_REG, 0x02, 0x00, 0x0A, 0x00, 0x00
  };
  size_t n;

  TEST_START();

  ADD_CRC(res);
  modbus_parser_init(parser, MODBUS_RESPONSE);

  n = modbus_parser_execute(parser, settings, res, sizeof(res));

  assert(n == sizeof(res));
  assert(parser->slave_addr == res[0]);
  assert(parser->function == res[1]);
  assert(parser->data_len == res[2]);

  TEST_SUCCESS();
}

void
test_write_single_coil(struct modbus_parser* parser,
                       struct modbus_parser_settings* settings)
{
  uint8_t res[] = { 0x11, MODBUS_FUNC_WRITE_COIL, 0x00, 0xAC, 0xFF, 0x00, 0x00,
                    0x00 };
  size_t n;

  TEST_START();

  ADD_CRC(res);
  modbus_parser_init(parser, MODBUS_RESPONSE);

  n = modbus_parser_execute(parser, settings, res, sizeof(res));

  assert(n == sizeof(res));
  assert(parser->slave_addr == res[0]);
  assert(parser->function == res[1]);
  assert(parser->addr == UINT16(res[2]));

  TEST_SUCCESS();
}

void
test_write_single_reg(struct modbus_parser* parser,
                      struct modbus_parser_settings* settings)
{
  uint8_t res[] = { 0x11, MODBUS_FUNC_WRITE_REG, 0x00, 0x01, 0x00, 0x03, 0x00,
                    0x00 };
  size_t n;

  TEST_START();

  ADD_CRC(res);
  modbus_parser_init(parser, MODBUS_RESPONSE);

  n = modbus_parser_execute(parser, settings, res, sizeof(res));

  assert(n == sizeof(res));
  assert(parser->slave_addr == res[0]);
  assert(parser->function == res[1]);
  assert(parser->addr == UINT16(res[2]));

  TEST_SUCCESS();
}

void
test_write_multiple_coil(struct modbus_parser* parser,
                         struct modbus_parser_settings* settings)
{
  uint8_t res[] = { 0x11, MODBUS_FUNC_WRITE_COILS, 0x00, 0x13, 0x00, 0x0A, 0x00,
                    0x00 };
  size_t n;

  TEST_START();

  ADD_CRC(res);
  modbus_parser_init(parser, MODBUS_RESPONSE);

  n = modbus_parser_execute(parser, settings, res, sizeof(res));

  assert(n == sizeof(res));
  assert(parser->slave_addr == res[0]);
  assert(parser->function == res[1]);
  assert(parser->addr == UINT16(res[2]));
  assert(parser->qty == UINT16(res[4]));

  TEST_SUCCESS();
}

void
test_write_multiple_reg(struct modbus_parser* parser,
                        struct modbus_parser_settings* settings)
{
  uint8_t res[] = { 0x11, MODBUS_FUNC_WRITE_REGS, 0x00, 0x01, 0x01, 0x02, 0x00,
                    0x00 };
  size_t n;

  TEST_START();

  ADD_CRC(res);
  modbus_parser_init(parser, MODBUS_RESPONSE);

  n = modbus_parser_execute(parser, settings, res, sizeof(res));

  assert(n == sizeof(res));
  assert(parser->slave_addr == res[0]);
  assert(parser->function == res[1]);
  assert(parser->addr == UINT16(res[2]));
  assert(parser->qty == UINT16(res[4]));

  TEST_SUCCESS();
}

void
test_crc_error(struct modbus_parser* parser,
               struct modbus_parser_settings* settings)
{
  /* Packet with bad CRC value */
  uint8_t res[] = { 0x11, MODBUS_FUNC_WRITE_REGS, 0x00, 0x01, 0x01, 0x02, 0x12,
                    0x34 };
  size_t n;

  TEST_START();

  modbus_parser_init(parser, MODBUS_RESPONSE);

  n = modbus_parser_execute(parser, settings, res, sizeof(res));

  assert(n == sizeof(res));
  assert(parser->slave_addr == res[0]);
  assert(parser->function == res[1]);
  assert(parser->addr == UINT16(res[2]));
  assert(parser->qty == UINT16(res[4]));
  assert(parser->errno != 0); /* TODO: check error value */

  TEST_SUCCESS();
}

int
main(void)
{
  struct modbus_parser parser;
  struct modbus_parser_settings settings;

  /* Initialization */
  modbus_parser_init(&parser, MODBUS_RESPONSE);
  modbus_parser_settings_init(&settings);

  /* Assign custom callbacks */
  settings.on_slave_addr = on_slave_addr;
  settings.on_function = on_function;
  settings.on_addr = on_addr;
  settings.on_qty = on_qty;
  settings.on_data_len = on_data_len;
  settings.on_data_end = on_data_end;
  settings.on_crc_error = on_crc_error;
  settings.on_complete = on_complete;

  test_read_coils(&parser, &settings);
  test_read_discrete_in(&parser, &settings);
  test_read_hold_reg(&parser, &settings);
  test_read_in_reg(&parser, &settings);
  test_write_single_coil(&parser, &settings);
  test_write_single_reg(&parser, &settings);
  test_write_multiple_coil(&parser, &settings);
  test_write_multiple_reg(&parser, &settings);
  test_crc_error(&parser, &settings);

  /*
  uint8_t data[] = { 0x11, MODBUS_FUNC_WRITE_REGS, 0x00, 0x01, 0x01, 0x02, 0x00,
                    0x00 };
  uint16_t crc = modbus_calc_crc(data, 6);
  printf("CRC: %04X\n", crc);

  crc = 0xFFFF;
  for (int i=0; i<6; i++) {
    modbus_crc_update(&crc, data[i]);
  }
  printf("CRC: %04X\n", crc);
  */
  return 0;
}
