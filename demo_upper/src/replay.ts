// Copyright 2026 Dev Team
//
// Licensed under the Apache License, Version 2.0.

import { promises as fs } from "node:fs";
import { McapIndexedReader } from "@mcap/core";
import { decompress } from "fzstd";
import { decodeAndPrint } from "./decode.ts";

interface ReplayOptions {
  file: string;
}

interface IReadable {
  size(): Promise<bigint>;
  read(offset: bigint, size: bigint): Promise<Uint8Array>;
}

/**
 * Replay messages from a local MCAP file.
 *
 * Uses McapIndexedReader for indexed access (supports seeking by time range)
 * and fzstd for zstd chunk decompression.
 */
export async function runReplay({ file }: ReplayOptions): Promise<void> {
  if (!file) {
    console.error("Error: --file <path> is required");
    process.exit(1);
  }

  console.log(`Reading MCAP file: ${file}`);
  const buffer = await fs.readFile(file);
  const data = new Uint8Array(buffer.buffer, buffer.byteOffset, buffer.byteLength);

  const readable: IReadable = {
    size: async () => BigInt(data.byteLength),
    read: async (offset: bigint, size: bigint) => {
      const start = Number(offset);
      return new Uint8Array(data.buffer, data.byteOffset + start, Number(size));
    },
  };

  const reader = await McapIndexedReader.Initialize({
    readable,
    decompressHandlers: {
      zstd: (compressedData: Uint8Array) => decompress(compressedData),
    },
  });

  console.log(`Channels: ${[...reader.channelsById.values()].map((c) => c.topic).join(", ")}`);

  let messageCount = 0;

  for await (const message of reader.readMessages()) {
    const channel = reader.channelsById.get(message.channelId);
    if (!channel) continue;
    const schema = reader.schemasById.get(channel.schemaId);
    if (!schema) continue;

    try {
      decodeAndPrint(schema.name, message.data, channel.topic);
      messageCount++;
    } catch (err) {
      console.error(
        `Failed to decode ${schema.name} on ${channel.topic} at logTime ${message.logTime}:`,
        (err as Error).message,
      );
    }
  }

  console.log(`\nReplayed ${messageCount} messages.`);
}
