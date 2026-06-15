/*
 * openai.js - Client OpenAI (JavaScript). Lê o contrato Rota do stdin
 * (saida de `astar_br --json`) e narra a rota com um modelo da OpenAI.
 *
 * Uso (rodar sob tsx para resolver o import de ./prompt.ts):
 *   ./astar_br --json 2211001 4314902 | npx tsx integ/openai.js
 *   ./astar_br --json 2211001 4314902 | npx tsx integ/openai.js --dry-run
 *
 * Chave: variavel de ambiente OPENAI_API_KEY.
 */
import OpenAI from "openai";
import { parseRota } from "./rota.ts";
import { buildPrompt } from "./prompt.ts";

process.stdout.on("error", (e) => { if (e && e.code === "EPIPE") process.exit(0); else throw e; });


const MODEL = "gpt-4o-mini"; // resumo curto e barato
const DRY_RUN = process.argv.includes("--dry-run");

async function lerStdin() {
  const chunks = [];
  for await (const c of process.stdin) chunks.push(c);
  return Buffer.concat(chunks).toString("utf8").trim();
}

async function main() {
  const raw = await lerStdin();
  if (!raw) {
    console.error("Sem entrada. Use: astar_br --json <o> <d> | npx tsx integ/openai.js");
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

  const apiKey = process.env.OPENAI_API_KEY;
  if (!apiKey) {
    console.error("Defina OPENAI_API_KEY no ambiente (export OPENAI_API_KEY=...).");
    process.exit(1);
  }

  const client = new OpenAI({ apiKey });
  const resp = await client.chat.completions.create({
    model: MODEL,
    max_tokens: 400,
    messages: [
      { role: "system", content: system },
      { role: "user", content: user },
    ],
  });
  console.log(resp.choices[0]?.message?.content ?? "(resposta vazia)");
}

main().catch((e) => {
  console.error("Erro:", e instanceof Error ? e.message : e);
  process.exit(1);
});
