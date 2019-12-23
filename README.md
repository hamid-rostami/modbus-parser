# Modbus protocol Sans-I/O implementation

Modbus protocol parser written in C. Design principles is very similar
to [http-parser](https://github.com/nodejs/http-parser) project.
It parses both requests and responses, Plus can generate modbus master
queries with simple API. It does not make any syscalls nor allocations,
not even I/O operation. it does not buffer data, it can be interrupted
at anytime depending on your architecture.

Features:

  * No dependencies
  * Decodes chunked encoding.
