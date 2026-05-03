/**
 * Prometheus Language Support – VS Code extension entry point
 *
 * Starts the prometheus-lsp Python server over stdio and wires it up
 * as a Language Client for all .prm files.
 */
"use strict";

const vscode = require("vscode");
const { LanguageClient, TransportKind } = require("vscode-languageclient/node");

let client;

function activate(context) {
  const config    = vscode.workspace.getConfiguration("prometheusLsp");
  const serverCmd = config.get("serverPath") || "prometheus-lsp";
  const binaryPath = config.get("binaryPath") || "";

  // Pass the prometheus binary path via env so the LSP server can find it
  const env = Object.assign({}, process.env);
  if (binaryPath) env.PROMETHEUS_BINARY = binaryPath;

  const serverOptions = {
    command: serverCmd,
    args: [],
    transport: TransportKind.stdio,
    options: { env },
  };

  const clientOptions = {
    documentSelector: [{ scheme: "file", language: "prometheus" }],
    synchronize: {
      fileEvents: vscode.workspace.createFileSystemWatcher("**/*.prm"),
    },
    traceOutputChannel: vscode.window.createOutputChannel("Prometheus LSP Trace"),
  };

  client = new LanguageClient(
    "prometheusLanguageServer",
    "Prometheus Language Server",
    serverOptions,
    clientOptions
  );

  client.start();
  context.subscriptions.push(client);
}

function deactivate() {
  if (client) return client.stop();
}

module.exports = { activate, deactivate };