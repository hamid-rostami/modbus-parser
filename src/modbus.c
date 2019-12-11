#include <string.h>

#include "modbus.h"

#define CALLBACK_NOTIFY(FOR)                                                   \
  do {                                                                         \
    if (settings->on_##FOR) {                                                  \
      if (settings->on_##FOR(parser) != 0) {                                   \
        parser->errno = 1;                                                     \
      }                                                                        \
    }                                                                          \
  } while (0)

void
modbus_parser_init(modbus_parser* parser, enum modbus_parser_type t)
{
  void* arg = parser->arg; /* preserve application data */
  memset(parser, 0, sizeof(*parser));
  parser->arg = arg;
  parser->type = t;
  parser->state = s_slave_addr;
}

void
modbus_parser_settings_init(modbus_parser_settings* settings)
{
  memset(settings, 0, sizeof(*settings));
}

const char*
modbus_func_str(enum modbus_func f)
{
  switch (f) {
#define XX(num, name, string)                                                  \
  case MODBUS_FUNC_##name:                                                     \
    return string;
    MODBUS_FUNC_MAP(XX)
#undef XX
    default:
      return "<unknown>";
  }
}

/*
static enum modbus_parser_state
state_after_function(modbus_func f)
{
  switch (f) {
    case MODBUS_FUNC_READ_COILS:
    case MODBUS_FUNC_READ_DISCRETE_IN:
    case MODBUS_FUNC_READ_HOLD_REG:
    case MODBUS_FUNC_READ_IN_REG:
      return s_len;

    case MODBUS_FUNC_WRITE_COIL:
    case MODBUS_FUNC_WRITE_REG:
      return s_single_addr_hi;

    case MODBUS_FUNC_WRITE_COILS:
    case MODBUS_FUNC_WRITE_REGS:
      return s_start_addr_hi;
  }
}
*/

static size_t
parse_query(modbus_parser* parser,
            const modbus_parser_settings* settings,
            const uint8_t* data,
            size_t len)
{
  return 0;
}

static size_t
parse_response(modbus_parser* parser,
               const modbus_parser_settings* settings,
               const uint8_t* data,
               size_t len)
{
  size_t nparsed = 0;

  for (int i = 0; i < len; i++) {
    if (parser->errno != 0)
      return nparsed;

    switch (parser->state) {
      case s_slave_addr:
        parser->slave_addr = *data;
        parser->state = s_func;
        CALLBACK_NOTIFY(slave_addr);
        break;

      case s_func:
        parser->function = *data;
        switch (parser->function) {
          case MODBUS_FUNC_READ_COILS:
          case MODBUS_FUNC_READ_DISCRETE_IN:
          case MODBUS_FUNC_READ_HOLD_REG:
          case MODBUS_FUNC_READ_IN_REG:
            parser->state = s_len;
            break;

          case MODBUS_FUNC_WRITE_COIL:
          case MODBUS_FUNC_WRITE_REG:
            parser->state = s_single_addr_hi;
            break;

          case MODBUS_FUNC_WRITE_COILS:
          case MODBUS_FUNC_WRITE_REGS:
            parser->state = s_start_addr_hi;
            break;
        }
        CALLBACK_NOTIFY(function);
        break;

      case s_len:
        parser->data_len = *data;
        parser->state = s_data;
        parser->data_cnt = 0;
        CALLBACK_NOTIFY(data_len);
        break;

      case s_single_addr_hi:
        parser->addr = (uint16_t)*data << 8;
        parser->state = s_single_addr_lo;
        break;

      case s_single_addr_lo:
        parser->addr += *data;
        parser->state = s_data;
        parser->data_cnt = 0;
        parser->data_len = 2;
        CALLBACK_NOTIFY(addr);
        break;

      case s_start_addr_hi:
        parser->addr = (uint16_t)*data << 8;
        parser->state = s_start_addr_lo;
        break;

      case s_start_addr_lo:
        parser->addr += *data;
        parser->state = s_qty_hi;
        CALLBACK_NOTIFY(addr);
        break;

      case s_qty_hi:
        parser->qty = (uint16_t)*data << 8;
        parser->state = s_qty_lo;
        break;

      case s_qty_lo:
        parser->qty += *data;
        parser->state = s_crc_hi;
        CALLBACK_NOTIFY(qty);
        break;

      case s_data:
        if (parser->data_cnt == 0) {
          /* start of data */
          parser->data = data;
          CALLBACK_NOTIFY(data_start);
        }

        parser->data_cnt++;
        if (parser->data_cnt == parser->data_len) {
          /* end data */
          CALLBACK_NOTIFY(data_end);
          parser->state = s_crc_hi;
        }
        break;

      case s_crc_hi:
        parser->frame_crc = (uint16_t)*data << 8;
        parser->state = s_crc_lo;
        break;

      case s_crc_lo:
        parser->frame_crc += *data;
        parser->state = s_complete;
        /* TODO: Calculate CRC */
        CALLBACK_NOTIFY(complete);
        break;

      case s_complete:
        return 0;

      default:
        return 0;
    }

    nparsed++;
    data++;
  }

  return nparsed;
}

size_t
modbus_parser_execute(modbus_parser* parser,
                      const modbus_parser_settings* settings,
                      const uint8_t* data,
                      size_t len)
{
  switch (parser->type) {
    case MODBUS_QUERY:
      return parse_query(parser, settings, data, len);
      break;

    case MODBUS_RESPONSE:
      return parse_response(parser, settings, data, len);
      break;
  }

  return 0;
}
