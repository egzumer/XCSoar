// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "Device.hpp"
#include "CRC16.hpp"
#include "Device/Error.hpp"
#include "Device/Port/Port.hpp"
#include "time/TimeoutClock.hpp"

[[gnu::pure]]
static const std::byte *
FindSpecial(const std::byte *const begin, const std::byte *const end)
{
  const std::byte *start = std::find(begin, end, FLARM::START_FRAME);
  const std::byte *escape = std::find(begin, end, FLARM::ESCAPE);
  return std::min(start, escape);
}

void
FLARM::SendEscaped(Port &port, const void *buffer, size_t length,
                   OperationEnvironment &env,
                   std::chrono::steady_clock::duration _timeout)
{
  assert(buffer != nullptr);
  assert(length > 0);

  const TimeoutClock timeout(_timeout);

  // Send data byte-by-byte including escaping
  const std::byte *p = (const std::byte *)buffer, *end = p + length;
  while (true) {
    const std::byte *special = FindSpecial(p, end);

    if (special > p) {
      /* bulk write of "harmless" characters */

      port.FullWrite({p, special}, env, timeout.GetRemainingOrZero());

      p = special;
    }

    if (p == end)
      break;

    // Check for bytes that need to be escaped and send
    // the appropriate replacements
    if (*p == START_FRAME) {
      port.Write(ESCAPE);
      port.Write(ESCAPE_START);
    } else if (*p == ESCAPE) {
      port.Write(ESCAPE);
      port.Write(ESCAPE_ESCAPE);
    } else
      // Otherwise just send the original byte
      port.Write(*p);

    p++;
  }
}

static std::byte *
ReceiveSomeUnescape(Port &port, std::span<std::byte> dest,
                    OperationEnvironment &env, const TimeoutClock timeout)
{
  /* read "length" bytes from the port, optimistically assuming that
     there are no escaped bytes */

  size_t nbytes = port.WaitAndRead(dest, env, timeout);

  /* unescape in-place */

  std::byte *p = dest.data();
  std::byte *end = dest.data() + nbytes;
  for (const std::byte *src = dest.data(); src != end;) {
    if (*src == FLARM::ESCAPE) {
      ++src;

      std::byte ch;
      if (src == end) {
        /* at the end of the buffer; need to read one more byte */
        port.WaitRead(env, timeout.GetRemainingOrZero());

        ch = (std::byte)port.ReadByte();
      } else
        ch = *src++;

      if (ch == FLARM::ESCAPE_START)
        *p++ = FLARM::START_FRAME;
      else if (ch == FLARM::ESCAPE_ESCAPE)
        *p++ = FLARM::ESCAPE;
      else
        /* unknown escape */
        return nullptr;
    } else
      /* "harmless" byte */
      *p++ = *src++;
  }

  /* return the current end position of the destination buffer; if
     there were escaped bytes, then this function must be called again
     to account for the escaping overhead */
  return p;
}

bool
FLARM::ReceiveEscaped(Port &port, void *buffer, size_t length,
                      OperationEnvironment &env,
                      std::chrono::steady_clock::duration _timeout)
{
  assert(buffer != nullptr);
  assert(length > 0);

  const TimeoutClock timeout(_timeout);

  // Receive data byte-by-byte including escaping until buffer is full
  std::byte *p = (std::byte *)buffer, *end = p + length;
  while (p < end) {
    p = ReceiveSomeUnescape(port, {p, std::size_t(end - p)},
                            env, timeout);
    if (p == nullptr)
      return false;
  }

  return true;
}

void
FlarmDevice::SendStartByte()
{
  port.Write(FLARM::START_FRAME);
}

inline void
FlarmDevice::WaitForStartByte(OperationEnvironment &env,
                              std::chrono::steady_clock::duration timeout)
{
  port.WaitForByte(FLARM::START_FRAME, env, timeout);
}

FLARM::FrameHeader
FLARM::PrepareFrameHeader(unsigned sequence_number, MessageType message_type,
                          const void *data, size_t length)
{
  assert((data != nullptr && length > 0) ||
         (data == nullptr && length == 0));

  FrameHeader header;
  header.length = 8 + length;
  header.version = 0;
  header.sequence_number = sequence_number++;
  header.type = (uint8_t)message_type;
  header.crc = CalculateCRC(header, data, length);
  return header;
}

FLARM::FrameHeader
FlarmDevice::PrepareFrameHeader(FLARM::MessageType message_type,
                                const void *data, size_t length)
{
  return FLARM::PrepareFrameHeader(sequence_number++, message_type,
                                   data, length);
}

void
FlarmDevice::SendFrameHeader(const FLARM::FrameHeader &header,
                             OperationEnvironment &env,
                             std::chrono::steady_clock::duration timeout)
{
  SendEscaped(&header, sizeof(header), env, timeout);
}

bool
FlarmDevice::ReceiveFrameHeader(FLARM::FrameHeader &header,
                                OperationEnvironment &env,
                                std::chrono::steady_clock::duration timeout)
{
  return ReceiveEscaped(&header, sizeof(header), env, timeout);
}

FLARM::MessageType
FlarmDevice::WaitForACKOrNACK(uint16_t sequence_number,
                              AllocatedArray<uint8_t> &data, uint16_t &length,
                              OperationEnvironment &env,
                              std::chrono::steady_clock::duration _timeout)
{
  const TimeoutClock timeout(_timeout);

  // Receive frames until timeout or expected frame found
  while (!timeout.HasExpired()) {
    // Wait until the next start byte comes around
    WaitForStartByte(env, timeout.GetRemainingOrZero());

    // Read the following FrameHeader
    FLARM::FrameHeader header;
    if (!ReceiveFrameHeader(header, env, timeout.GetRemainingOrZero()))
      continue;

    // Read and check length of the FrameHeader
    length = header.length;
    if (length <= sizeof(header))
      continue;

    // Calculate payload length
    length -= sizeof(header);

    // Read payload and check length
    data.GrowDiscard(length);
    if (!ReceiveEscaped(data.data(), length,
                        env, timeout.GetRemainingOrZero()))
      continue;

    // Verify CRC
    if (header.crc != FLARM::CalculateCRC(header, data.data(), length))
      continue;

    // Check message type
    if (header.type != FLARM::MT_ACK && header.type != FLARM::MT_NACK)
      continue;

    // Check payload length
    if (length < 2)
      continue;

    // Check whether the received ACK is for the right sequence number
    if (FromLE16(*((const uint16_t *)(const void *)data.data())) ==
        sequence_number)
      return (FLARM::MessageType)header.type;
  }

  return FLARM::MT_ERROR;
}

FLARM::MessageType
FlarmDevice::WaitForACKOrNACK(uint16_t sequence_number,
                              OperationEnvironment &env,
                              std::chrono::steady_clock::duration timeout)
{
  AllocatedArray<uint8_t> data;
  uint16_t length;
  return WaitForACKOrNACK(sequence_number, data, length, env, timeout);
}

bool
FlarmDevice::WaitForACK(uint16_t sequence_number,
                        OperationEnvironment &env,
                        std::chrono::steady_clock::duration timeout)
{
  return WaitForACKOrNACK(sequence_number, env, timeout) == FLARM::MT_ACK;
}

bool
FlarmDevice::BinaryPing(OperationEnvironment &env,
                        std::chrono::steady_clock::duration _timeout)
try {
  const TimeoutClock timeout(_timeout);

  // Create header for sending a binary ping request
  FLARM::FrameHeader header = PrepareFrameHeader(FLARM::MT_PING);

  // Send request and wait for positive answer

  SendStartByte();
  SendFrameHeader(header, env, timeout.GetRemainingOrZero());
  return WaitForACK(header.sequence_number, env, timeout.GetRemainingOrZero());
} catch (const DeviceTimeout &) {
  return false;
}

void
FlarmDevice::BinaryReset(OperationEnvironment &env,
                         std::chrono::steady_clock::duration _timeout)
{
  TimeoutClock timeout(_timeout);

  // Create header for sending a binary reset request
  FLARM::FrameHeader header = PrepareFrameHeader(FLARM::MT_EXIT);

  // Send request and wait for positive answer
  SendStartByte();
  SendFrameHeader(header, env, timeout.GetRemainingOrZero());
}
