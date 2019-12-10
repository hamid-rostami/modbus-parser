#include <string.h>

#include "modbus.h"

void
modbus_parser_init(modbus_parser* parser, enum modbus_parser_type t)
{
  void* arg = parser->arg; /* preserve application data */
  memset(parser, 0, sizeof(*parser));
  parser->arg = arg;
  parser->type = t;
}

void
mosbus_parser_settings_init(modbus_parser_settings* settings)
{
  memset(settings, 0, sizeof(*settings));
}

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
  return 0;
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
