// Copyright 2026 Dev Team
//
// Licensed under the Apache License, Version 2.0.

import { fromBinary } from "@bufbuild/protobuf";
import type { TopicMessage } from "../generated/proto/bridge/topic_message_pb.ts";
import { TopicMessageSchema } from "../generated/proto/bridge/topic_message_pb.ts";
import type { Log } from "../generated/proto/foxglove/Log_pb.ts";
import { LogSchema, Log_Level } from "../generated/proto/foxglove/Log_pb.ts";

const FOXGLOVE_LOG_LEVELS: Record<number, string> = {
  [Log_Level.UNKNOWN]: "UNKNOWN",
  [Log_Level.DEBUG]: "DEBUG",
  [Log_Level.INFO]: "INFO",
  [Log_Level.WARNING]: "WARN",
  [Log_Level.ERROR]: "ERROR",
  [Log_Level.FATAL]: "FATAL",
};

type Decoder = (data: ArrayLike<number>) => TopicMessage | Log;

/**
 * Schema-name to decoder registry.
 */
const DECODERS: Record<string, Decoder> = {
  "bridge.TopicMessage": (data) => fromBinary(TopicMessageSchema, new Uint8Array(data)),
  "foxglove.Log": (data) => fromBinary(LogSchema, new Uint8Array(data)),
};

/**
 * Decode a protobuf message given its schema name and raw bytes.
 * Returns undefined if the schema is not recognized.
 */
export function decodeMessage(schemaName: string, data: ArrayLike<number>): TopicMessage | Log | undefined {
  const decoder = DECODERS[schemaName];
  return decoder ? decoder(data) : undefined;
}

/**
 * Print a TopicMessage to stdout.
 */
export function printTopicMessage(msg: TopicMessage, topic: string): void {
  const tsSec = Number(msg.timestampNs) / 1e9;
  console.log(`  [${topic}] ${msg.sender} #${msg.sequence}: "${msg.content}" (t=${tsSec.toFixed(3)})`);
}

/**
 * Print a foxglove.Log message to stdout.
 */
export function printFoxgloveLog(msg: Log): void {
  const level = FOXGLOVE_LOG_LEVELS[msg.level] || `LVL${msg.level}`;
  const ts = msg.timestamp;
  const tsSec = ts ? Number(ts.seconds) + ts.nanos / 1e9 : 0;
  console.log(`  [${level}] ${msg.name}: ${msg.message} (t=${tsSec.toFixed(3)})`);
}

/**
 * Decode and print a message dispatched by schema name.
 */
export function decodeAndPrint(schemaName: string, data: ArrayLike<number>, topic: string): void {
  const bytes = new Uint8Array(data);

  switch (schemaName) {
    case "bridge.TopicMessage":
      printTopicMessage(fromBinary(TopicMessageSchema, bytes), topic);
      break;
    case "foxglove.Log":
      printFoxgloveLog(fromBinary(LogSchema, bytes));
      break;
    default:
      console.log(`  [unknown schema: ${schemaName}] ${data.length} bytes on ${topic}`);
  }
}
