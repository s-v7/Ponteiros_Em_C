/*
 * anthropic.ts - Client Anthropic (TypeScript). Lê o contrato Rota do stdin
 * (saida de `astar_br --json`) e narra a rota com Claude.
 * Chave: variavel de ambiente ANTHROPIC_API_KEY .
 */
import Anthropic from "@anthropic-ai/sdk";
import { parseRota } from "./rota.ts";
import { buildPrompt } from "./prompt.ts";

process.stdout.on("error", (e: NodeJS.ErrnoException) => { if (e && e.code === "EPIPE") process.exit(0); else throw e; });

const MODEL = "claude-haiku-4-5-20251001"; // resumo curto: Haiku
const DRY_RUN = process.argv.includes("--dry-run");

async function lerStdin(): Promise<string> {
  const chunks: Buffer[] = [];
  for await (const c of process.stdin) chunks.push(c as Buffer);
  return Buffer.concat(chunks).toString("utf8").trim();
}

async function main(): Promise<void> {
  const raw = await lerStdin();
  if (!raw) {
    console.error("Sem entrada. Use: astar_br --json <o> <d> | npx tsx integ/anthropic.ts");
    process.exit(1);
  }

  const rota = parseRota(raw);
  const { system, user } = buildPrompt(rota);

  if (DRY_RUN) {
    console.log("=== DRY-RUN (sem chamar a API) ===");
    console.log("--- system ---\n" + system);
    console.log("\n--- user ---\n" + user);
    return;
  }

  const apiKey = process.env.ANTHROPIC_API_KEY;
  if (!apiKey) {
    console.error("Defina ANTHROPIC_API_KEY no ambiente (export ANTHROPIC_API_KEY=...).");
    process.exit(1);
  }

  const client = new Anthropic({ apiKey });
  const resp = await client.messages.create({
    model: MODEL,
    max_tokens: 400,
    system,
    messages: [{ role: "user", content: user }],
  });

  const texto = resp.content
    .filter((b): b is Anthropic.TextBlock => b.type === "text")
    .map((b) => b.text)
    .join("\n");
  console.log(texto);
}

main().catch((e) => {
  console.error("Erro:", e instanceof Error ? e.message : e);
  process.exit(1);
});
