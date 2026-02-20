// Copyright 2026 Dev Team
//
// Licensed under the Apache License, Version 2.0.

import { program } from "commander";
import { runLive } from "./live.ts";
import { runReplay } from "./replay.ts";

program
  .name("demo-upper")
  .description("Upper-host CLI for bridge data");

program
  .command("live")
  .description("Subscribe to live MCAP chunks via Zenoh")
  .option("--host <host>", "zenoh-bridge-remote-api host", "localhost")
  .option("--port <port>", "zenoh-bridge-remote-api WebSocket port", "10000")
  .action(runLive);

program
  .command("replay")
  .description("Replay messages from an MCAP file")
  .requiredOption("--file <path>", "Path to MCAP file")
  .action(runReplay);

program.parse();
