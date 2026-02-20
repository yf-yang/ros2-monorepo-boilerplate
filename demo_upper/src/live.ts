// Copyright 2026 Dev Team
//
// Licensed under the Apache License, Version 2.0.

import { open as openZenohSession, KeyExpr, Config } from "@eclipse-zenoh/zenoh-ts";
import { McapStreamReader } from "@mcap/core";
import { decompress } from "fzstd";
import type { TypedMcapRecord } from "@mcap/core";
import { decodeAndPrint } from "./decode.ts";

interface LiveOptions {
  host?: string;
  port?: string | number;
}

/**
 * Subscribe to live collector data via Zenoh.
 *
 * The collector node publishes self-contained mini-MCAP files as raw bytes
 * on Zenoh key bridge/mcap every second.
 */
export async function runLive({ host = "localhost", port = 10000 }: LiveOptions = {}): Promise<void> {
  const locator = `ws/${host}:${port}`;
  console.log(`Connecting to zenoh-bridge-remote-api at ${locator} ...`);

  const config = new Config(locator);
  const session = await openZenohSession(config);
  console.log("Connected. Subscribing to bridge/mcap ...");

  const keyExpr = new KeyExpr("bridge/mcap");
  await session.declareSubscriber(keyExpr, {
    handler: (sample) => {
      try {
        const mcapBytes = sample.payload().toBytes();
        processMiniMcap(mcapBytes);
      } catch (err) {
        console.error("Failed to process mini-MCAP:", (err as Error).message);
      }
    },
  });

  console.log("Listening for MCAP chunks. Press Ctrl+C to stop.");

  // Keep alive until interrupted
  await new Promise<never>(() => {});
}

/**
 * Parse a self-contained mini-MCAP and decode/print all messages.
 */
function processMiniMcap(data: Uint8Array): void {
  const reader = new McapStreamReader({
    decompressHandlers: {
      zstd: (compressedData: Uint8Array) => decompress(compressedData),
    },
  });
  reader.append(data);

  const schemas = new Map<number, TypedMcapRecord & { type: "Schema" }>();
  const channels = new Map<number, TypedMcapRecord & { type: "Channel" }>();
  let record: ReturnType<typeof reader.nextRecord>;

  while ((record = reader.nextRecord()) !== undefined) {
    switch (record.type) {
      case "Schema":
        schemas.set(record.id, record);
        break;
      case "Channel":
        channels.set(record.id, record);
        break;
      case "Message": {
        const channel = channels.get(record.channelId);
        if (!channel) break;
        const schema = schemas.get(channel.schemaId);
        if (!schema) break;

        try {
          decodeAndPrint(schema.name, record.data, channel.topic);
        } catch (err) {
          console.error(`Failed to decode ${schema.name} on ${channel.topic}:`, (err as Error).message);
        }
        break;
      }
    }
  }
}
